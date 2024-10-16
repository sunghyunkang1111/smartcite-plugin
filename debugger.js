/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Portions created by the Adobe Systems Incorporated are Copyright (C) 2004
 * Adobe Systems Incorporated. All Rights Reserved.
 *
 * Contributor(s):
 *   Adobe Systems Incorporated
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */


var acrobatDebugger = new Debugger();

acrobatDebugger.addAllGlobalsAsDebuggees();

/***************************************************************************/

// The following code "exports" any strings in the list into the current scope.
var msgStrings = new Array(
	"IDS_STARTUP_DEBUGGER_MSG",
	"IDS_INVALID_BREAKPOINT", 
	"IDS_EXCEPTION_INLINE",
	"IDS_UNABLE_FINDSOURCE",
	"IDS_TOP_LEVEL",
	"IDS_INVALID_BPLINE",
	"IDS_STACK_STRING",
	"IDS_CANNOT_EVALWATCH");

for(var n = 0; n < msgStrings.length; n++)
	eval(msgStrings[n] + " = " + getString("EScript", msgStrings[n]).toSource());


/*************************   Stepping   *************************/



// Current Line to track progress while stepping
var currLine = 1;

function onStepHandler() {
	// This function can ONLY be called once handler is set in step() method

	currentFrameIndex = 0;
	buildStack();
	
	var sc = this.script;
	var offset = this.offset;
	var line = sc.getOffsetLine(offset);
	var file = sc.url;
	
	if(line == currLine) {
		// Not progressed enough. Return from this call and wait for next one
		return undefined;
	}
	
	// Haven't returned => progressed
	unsuspend();
	
	var bps = sc.getBreakpoints(offset);
	if (bps.length > 0) {   // Breakpoint exists
		resume();
	} else {	
		resumeCode = -1;
		openDebugger();
		selectLine(file, line, 0);
		suspendThread();
	}
};

function onPopHandler() {
	// This function can ONLY be called once handler is set in step() method

	currentFrameIndex = 0;
	buildStack();
	
	var sc = this.script;
	var offset = this.offset;
	var line = sc.getOffsetLine(offset);
	var file = sc.url;
	currLine = line;
	
	unsuspend();
	
	// Set previous frames's onStep handler
	var prevFrame = this.older;
	if(prevFrame != null)
		prevFrame.onStep = onStepHandler;

};

function onStepInstructionHandler() {
	// This function can ONLY be called once handler is set in step() method

	currentFrameIndex = 0;
	buildStack();
	
	var sc = this.script;
	var offset = this.offset;
	var line = sc.getOffsetLine(offset);
	var file = sc.url;
		
	unsuspend();
	
	resumeCode = -1;
	openDebugger();
	selectLine(file, line, 0);
	suspendThread();
};

function step(type) {
	// type 0 => Step byte-code
	// type 1 => Step in   (step)
	// type 2 => Step over (next)
	// type 3 => Step out  (pop)
	
	if(type == 1) {
		// Stepping in : 
		//		So new frame should have the handler set. 
		//		Otherwise not.
		acrobatDebugger.onEnterFrame = function (frame) {
			frame.onStep = onStepHandler;
		}
	}
	
	var currentFrame = acrobatDebugger.getNewestFrame();
	while(currentFrame.type == "eval") {
		currentFrame = currentFrame.older;
	}
	if(type == 1 || type == 2) {
		// Stepping over OR Stepping in :
		//		Set handler for current plus all parent frames
		currentFrame.onStep = onStepHandler;	
		var parentFrame = currentFrame.older;
		while(parentFrame) {
			parentFrame.onStep = onStepHandler;
			parentFrame = parentFrame.older;
		}
	}
	
	if(type == 3) {
		// Stepping out :
		currentFrame.onPop = onPopHandler;
	}
	
	if(type == 0) {
		acrobatDebugger.onEnterFrame = function (frame) {
			frame.onStep = onStepInstructionHandler;
		}
		currentFrame.onStep = onStepInstructionHandler;
		var parentFrame = currentFrame.older;
		while(parentFrame) {
			parentFrame.onStep = onStepInstructionHandler;
			parentFrame = parentFrame.older;
		}
	}
	
	// Get the current line to track progress
	var sc 		= currentFrame.script;
	var offset 	= currentFrame.offset;
	var line 	= sc.getOffsetLine(offset);
	currLine = line;
	
	resume();
}

function disableAllHandlers() {
	acrobatDebugger.onEnterFrame = undefined;
	var cFrame = acrobatDebugger.getNewestFrame();
	while(cFrame != null) {
		cFrame.onStep = undefined;
		cFrame.onPop = undefined;
		cFrame = cFrame.older;
	}
}

/*************************   Interrupt Handler   *************************/


function interruptHandler() { 
	// Very similar to onStepHandler and used as step handler of interrupt icon

	currentFrameIndex = 0;
	buildStack();
	
	unsuspend();
	
	var sc = this.script;
	var offset = this.offset;
	var line = sc.getOffsetLine(offset);
	var file = sc.url;
	
	acrobatDebugger.onEnterFrame = undefined;
	this.onStep = undefined;
	
	resumeCode = -1;
	openDebugger();
	selectLine(file, line, 0);
	suspendThread();
	
	currLine = line;
};

function unsuspend() {
	disableAllHandlers();
	clearInterrupt();
}

function suspend() {
	disableAllHandlers();
	
	acrobatDebugger.onEnterFrame = function (frame) {
		frame.onStep = interruptHandler;
	}
	var currentFrame = acrobatDebugger.getNewestFrame();
	var tFrame = currentFrame;
	while(tFrame) {
		tFrame.onStep = interruptHandler;
		tFrame = tFrame.older;
	}
	
	setInterrupt();
}

// We need to maintain a stack instead of scalar for the case of consecutive save calls
var saveHandlerStack = [];

// Calls to saveAllHandlers() and restoreAllHandlers() should be matched 1-1
function saveAllHandlers() {
	var saveHandlerObject = { onEnterFrameHandler : undefined, onStepHandler :[], onPopHandler :[],};
	saveHandlerObject.onEnterFrameHandler = acrobatDebugger.onEnterFrame;
	var cFrame = acrobatDebugger.getNewestFrame();
	while(cFrame != null) {
		saveHandlerObject.onStepHandler.push(cFrame.onStep);
		saveHandlerObject.onPopHandler.push(cFrame.onPop);
		cFrame = cFrame.older;
	}
	saveHandlerStack.push(saveHandlerObject);
}

function restoreAllHandlers() {	
	var saveHandlerObject = saveHandlerStack.pop();
	acrobatDebugger.onEnterFrame = saveHandlerObject.onEnterFrameHandler;
	var cFrame = acrobatDebugger.getNewestFrame();
	var i = 0;
	while(cFrame != null && i < saveHandlerObject.onStepHandler.length) {
		cFrame.onStep = saveHandlerObject.onStepHandler[i];
		cFrame.onPop = saveHandlerObject.onPopHandler[i];
		cFrame = cFrame.older;
		i++;
	}
	saveHandlerStack.pop();
}

/*************************   Script & Function Listing   *************************/

function scriptExists(fileName) {
	if(scriptList[fileName])
		return true;
	else
		return false;
}

function listChildScriptFunctions(script) {
	var chScripts = script.getChildScripts();
	for( var i=0; i < chScripts.length ; i++) {
		var chScript = chScripts[i];		
		printFunction(chScript.displayName, chScript.startLine);
		listChildScriptFunctions(chScript);
	}
}
function listScriptFunctions(fileName) {
	var script = scriptList[fileName];
	if(script) {
		var childScripts = script.getChildScripts();
		if(childScripts.length > 0) {
			if(script.staticLevel == 0) {
				printFunction(IDS_TOP_LEVEL, script.startLine);
			}
			listChildScriptFunctions(script);
		}
	}
}

/*************************   Breakpoints   *************************/

function findScriptInSubtreeForLine(script, lineNo) {
	var sc = script;
	var offsets = sc.getLineOffsets(lineNo);
	if(offsets.length >0)
		return sc;
	
	var chScripts = sc.getChildScripts();
	for(var i=0; i<chScripts.length ; i++) {
		var chScript = chScripts[i];
		var foundScript = findScriptInSubtreeForLine(chScript, lineNo);
		if(foundScript)
			return foundScript;
	}
	return null;
}	

acrobatDebugger.onNewScript = function(script) {
	// Following check is a safe-guard to guarantee that we update scriptList and bpList and breakpoints only for top-level scripts
	if(script.staticLevel == 0 && script.sourceLength == script.source.text.length && script.sourceStart == 0) {
		if(scriptList[script.url] != script) {
			scriptList[script.url] = script;
		}
		
		for(var i=0 ; i < bpList.length ; i++ ) {
			if(script.url == bpList[i].fileName) {
				var sc = findScriptInSubtreeForLine(script, bpList[i].lineNum);
				if(sc != null) {
					bpList[i].script = sc;
					bpList[i].offsets = sc.getLineOffsets(bpList[i].lineNum);
					
					for(var j=0; j < bpList[i].offsets.length ; j++) {
						bpList[i].script.setBreakpoint(bpList[i].offsets[j], bpHandler);
					}
					
				} else {
					delete bpList[i];
					bpList.splice(i, 1);
					i--;
					saveBreakpoints();
				}
			}
		}
	}
}

function updateBreakpointsForScript(fileName) {	
	for(var i = 0; i < bpList.length; i++) {
		if (bpList[i].fileName == fileName) {
			updateBreakpoint(true, bpList[i].lineNum, i, fileName, bpList[i].condition);
			
			for(var j=0; j < bpList[i].offsets.length ; j++ ) {
				var bps = bpList[i].script.getBreakpoints(bpList[i].offsets[j]);
				if(bps.length == 0) // No existing breakpoint, then set
					bpList[i].script.setBreakpoint(bpList[i].offsets[j], bpHandler);
			}
		}
	}
}

function canSetBreakpointInInternalScripts(script, lineNo) {
	var sc = script;
	var offsets = sc.getLineOffsets(lineNo);
	if(offsets.length >0)
		return true;
	
	var chScripts = sc.getChildScripts();
	for(var i=0; i<chScripts.length ; i++) {
		var chScript = chScripts[i];
		var found = canSetBreakpointInInternalScripts(chScript, lineNo);
		if(found)
			return true;
	}
	return false;
}

function canSetBreakpoint(fileName, lineNum) {
	if(scriptList[fileName]) {
		var sc = scriptList[fileName];
		return canSetBreakpointInInternalScripts(sc,lineNum);
	}
	return false;
}

// File name to Script 1-1 map 
// Needed for canSetBreakpoint as we set BP icons before setting BP on actual script
var scriptList = {};

// Data structure to store array of breakpoints
var bpList = [];

function Breakpoint(pddHandle, fileName, lineNum, script, offsets, condition) {
	this.pddHandle = pddHandle;
    this.fileName = fileName;
    this.lineNum = lineNum;
	this.script = script;
	this.offsets = offsets;
    this.condition = condition;
	this.isSet = false;
}

function getBPIndex(fileName, lineNum) {
    for(var i = 0; i < bpList.length; i++)
        if (bpList[i].fileName == fileName && bpList[i].lineNum == lineNum)
            return i;
    return -1;
}

function pcMapExists(fileName) {
	if(scriptList[fileName]) 
		return true;
	else
		return false;
}

// Breakpoint handler
const bpHandler = {
	hit: function(frame) 
	{	
		currentFrameIndex = 0;
		buildStack();
		
		unsuspend();
		
		var sc = frame.script;
		var offset = frame.offset;
		var line = sc.getOffsetLine(offset);
		var file = sc.url;
		
		//BP Condition
		var bpIndex = getBPIndex(file, line);
		var conditionVal = debugEval(bpList[bpIndex].condition);
		if(conditionVal == false || conditionVal == undefined || conditionVal == null || conditionVal == 0)
			return;
		
		resumeCode = -1;
		openDebugger();
		selectLine(file, line, 0);
		suspendThread();
	}
};

function setBreakpoint(pddHandle, fileName, lineNum, condition) {
	var bpCreate = canSetBreakpoint(fileName, lineNum);
	if(bpCreate) {   // Line number is valid for breakpoint
		var bpIndex = getBPIndex(fileName, lineNum);
		if (bpIndex == -1) {   // Breakpoint does not exist already
			if(scriptList[fileName]) {   // Script is in debugger window
				var sc = scriptList[fileName];
				sc = findScriptInSubtreeForLine(sc, lineNum);
				if(sc == null)
					return;
				
				var offsets = sc.getLineOffsets(lineNum);
				for(var j=0; j<offsets.length ; j++) {
					var offset = offsets[j];
					sc.setBreakpoint(offset, bpHandler);				
				}
				if(offsets.length > 0) {
					var theCondition = (condition == undefined) ? "true" : condition;
					bpList[bpList.length] = new Breakpoint(pddHandle, fileName, lineNum, sc, offsets, theCondition);
					
					updateBreakpoint(true, lineNum, bpList.length-1, fileName, theCondition);
					saveBreakpoints();
				}
			}
		}
	}
}

function clearBreakpoint(fileName, lineNum) {
	var bpCreate = canSetBreakpoint(fileName, lineNum);
	if(bpCreate) {   // Line number is valid for breakpoint
		var bpIndex = getBPIndex(fileName, lineNum);
		if (bpIndex != -1) {   // Breakpoint exists
			if(scriptList[fileName]) {   // Script is in debugger window
				var sc = scriptList[fileName];
				sc = findScriptInSubtreeForLine(sc, lineNum);
				if(sc == null)
					return;
				
				var offsets = sc.getLineOffsets(lineNum);
				for(var j=0; j<offsets.length ; j++) {
					var offset = offsets[j];
					sc.clearAllBreakpoints(offset);
				}
				if(offsets.length>0) {
					delete bpList[bpIndex];
					bpList.splice(bpIndex,1);

					updateBreakpoint(false, lineNum, bpIndex, fileName, "");
					saveBreakpoints();
				}
			}
		}
	}
}

function toggleBreakpoint(pddHandle, fileName, lineNum, condition) {
    var bpIndex = getBPIndex(fileName, lineNum);
    if (bpIndex == -1)
		setBreakpoint(pddHandle, fileName, lineNum, condition);
	else
	    clearBreakpoint(fileName, lineNum);
}

// This function gets called at document close
function clearBreakpointsForDoc(pddHandle) {
	var fileNames = [];
	// Clean-up Breakpoint list
	for(var i = 0; i < bpList.length; i++) {
		if (bpList[i].pddHandle == pddHandle) {
			updateBreakpoint(false, bpList[i].lineNum, i, bpList[i].fileName, bpList[i].condition);
			fileNames.push(bpList[i].fileName);
			delete bpList[i];
			bpList.splice(i, 1);
			i--;
			saveBreakpoints();
		}
	}
	
	// Clean-up Script list
	for(var fileName in fileNames) {
		if(scriptList[fileName]) {
			scriptList[fileName] = undefined;
		}
	}
}

function changeBPCondition(i, newCondition) {
    if (i >= 0 && i < bpList.length && newCondition) {
	    bpList[i].condition = newCondition;
        updateBreakpoint(true, bpList[i].lineNum, i, bpList[i].fileName, bpList[i].condition);
		saveBreakpoints();
	}
}

/*******************   Save Breakpoints in Document   *******************/

function enumBreakpoints() {
	if(bpList.length > 0) {
		for(var i = 0; i < bpList.length; i++)
			regBreakpoint(i, bpList[i].fileName, bpList[i].lineNum, bpList[i].condition);
	} else {
		regBreakpoint(-1); // Clear last breakpoint
	}
}

function setBreakpoints(pddHandle, bpArray) {
    for(var i = 0; i < bpArray.length; i++) {
		var fileName = bpArray[i].fileName;
		if (fileName.substr(fileName.length - 3) != ".js") {
			if(pcMapExists(fileName) == false) {
				compileScript(fileName);
			}
			setBreakpoint(pddHandle, fileName, bpArray[i].lineNum, bpArray[i].condition);
		}
	}
}


/*************************   Inspect State   *************************/

// Current frame to show variables and text of that fame. 
// User can select from UI. It should be top frame while hitting a breakpoint and stepping.
var currentFrameIndex;

// The stack data structure shown in debugger UI
var jsStack = null;

function selectFrame(frameNum) {
    currentFrameIndex = frameNum;
	var currFrame = jsStack[currentFrameIndex];
	if(currFrame) {
		selectLine(currFrame.file, currFrame.lineno, frameNum);
	} else {
		// No Current frame!!
	}
}

function JSDFrame(frame) {
    this.frame = frame;
	this.file = frame.script.url;
	this.lineno = frame.script.getOffsetLine(frame.offset);
}

function buildStack() {
    jsStack = [];
	
	var currentFrame = acrobatDebugger.getNewestFrame();	
	while(currentFrame != null) {
		jsStack.push(new JSDFrame(currentFrame));
		currentFrame = currentFrame.older;
	}
}

function JSDProperty(propValue, readOnly) {
    this.value = propValue;
	this.type = ( (typeof propValue  == "object")	? "o" : "" ) +
				( (typeof propValue  == "number")	?  ( Number.isInteger(propValue) ? "i" : "d") : "" ) +
				( (typeof propValue  == "string")	? "s" : "" ) +
				( (typeof propValue  == "boolean")	? "b" : "" ) +
				( (typeof propValue  == "undefined")? "u" : "" ) +
				( readOnly				       		? "r" : "w" );
				
	if(propValue) {
		if(typeof propValue  == "object") {
			this.valString = propValue.unsafeDereference().toString();
		} else {
			this.valString = propValue.toString();
		}
	} else {
		this.valString = new String(propValue);
	}
	// If this is a null value, display empty string instead
	if(this.valString == null)
	    this.valString = "";
}

// Used in listRecursive. To detect change of frame and clearing the inspector window
// The expansion feature requires that we do not clear the window if frame is not changed
var curScriptFrameThis = null;
var curScriptFrameScope = null;

function listRecursive(bThis) {	
	if (jsStack) {
		var currFrame = jsStack[currentFrameIndex].frame;

		if(!currFrame.live)
			return;
		
		var callObjHandle;
		
		if(bThis) {
			callObjHandle = currFrame.this;
			if(curScriptFrameThis != currFrame) {
				clearInspector();
				curScriptFrameThis = currFrame;
			}
		} else {
			callObjHandle = currFrame.environment;
			if(curScriptFrameScope != currFrame) {
				clearInspector();
				curScriptFrameScope = currFrame;
			}
		}	
		
		// We do not want to halt due to any debuggee calls from displayRecursive(). 
		// So saving, disabling and then restoring all execution handlers
		saveAllHandlers();
		disableAllHandlers();
		displayRecursive(callObjHandle,0);
		restoreAllHandlers();
	} else {
		clearInspector();
	}
}

function displayRecursive(callObjHandle,tab) {	
	if(callObjHandle) {
		var currFrame = jsStack[currentFrameIndex].frame;
				
		if(callObjHandle instanceof Debugger.Environment) {
			var names = callObjHandle.names().sort();
			
			var entryID = 0;
			for(var i = 0; i < names.length ; i++) {
				var name = names[i];
				var prop = callObjHandle.getVariable(name);
				
				var isReadOnly = false; // TODO : check mutability of local variables
								
				if(typeof prop == "object") {
					if(prop && prop.callable){
						// Do not show functions
						continue;
					} 

					var propJSDObj = new JSDProperty(prop, isReadOnly);
					var isExpanded = printInspector(tab, entryID, propJSDObj.type, name, propJSDObj.valString);
					if( isExpanded && propJSDObj.type[0] == "o") {
						displayRecursive(prop, tab+1);
					}
				} else {
					var propJSDObj = new JSDProperty(prop, isReadOnly);
					printInspector(tab, entryID, propJSDObj.type, name, propJSDObj.valString);
				}
				entryID++;
			}
		} 
		else if(callObjHandle instanceof Debugger.Object) {
			
			var protoValue = callObjHandle.proto;
			if(protoValue) {
				var protoJSDObj = new JSDProperty(protoValue, false);
				var isExpanded = printInspector(tab, 0, protoJSDObj.type, "[[Prototype]]", protoJSDObj.valString);
				if( isExpanded && protoJSDObj.type[0] == "o") {
					displayRecursive(protoValue, tab+1);
				}
			}
			
			var names = callObjHandle.getOwnPropertyNames().sort();
			var entryID = 1;
			for(var i = 0; i < names.length ; i++) {
				var name = names[i];
				var pd = callObjHandle.getOwnPropertyDescriptor(name);
				var prop = pd.value;
				
				if(typeof prop == "object") {
					if(prop && prop.callable){
						// Do not show functions
						continue;
					} 
					var propJSDObj = new JSDProperty(prop, !pd.writable);
					var isExpanded = printInspector(tab, entryID, propJSDObj.type, name, propJSDObj.valString);
					if( isExpanded && propJSDObj.type[0] == "o") {
						displayRecursive(prop, tab+1);
					}
				} else {
					var propJSDObj = new JSDProperty(prop, !pd.writable);
					printInspector(tab, entryID, propJSDObj.type, name, propJSDObj.valString);
				}
				entryID++;
			}
		}
	}
}

function where() {
	if (jsStack) {
    	for(var i = 0; i < jsStack.length; i++) {
			var displayString = IDS_STACK_STRING.replace('%d', jsStack[i].lineno);
			var funcName = (jsStack[i].frame.callee) ? jsStack[i].frame.callee.displayName : IDS_TOP_LEVEL;
			displayString = displayString.replace('%s', funcName);
        	printStack(displayString);
		}
    }
}

/*************************   Exception Handling  *************************/

const TMODE_IGNORE = 0;
const TMODE_TRACE = 1;
const TMODE_BREAK = 2;

acrobatDebugger.onExceptionUnwind = function (frame, ex) {
	
	var frameUrl = frame.script && frame.script.url;
	if (frameUrl === "self-hosted" )
        return undefined;
	
	var tMode = getThrowMode();
	switch (tMode) {
		case TMODE_IGNORE:
			return undefined;
		case TMODE_TRACE:
		case TMODE_BREAK:
			var frameName = (frame.callee) ? frame.callee.displayName : IDS_TOP_LEVEL;
			var line = frame.script.getOffsetLine(frame.offset);
			
			var displayString = IDS_EXCEPTION_INLINE.replace('%d', line);
			displayString = displayString.replace('%s', frameName);
			displayString = displayString.replace('%s', frameUrl);
			printConsole(displayString);
			
			if(tMode == TMODE_BREAK) {
				interruptHandler.call(frame);
			}
			return undefined;
		default:
			return false;
	}
	return undefined;
}

acrobatDebugger.uncaughtExceptionHook = function (ex) {
	return undefined;
};

/*************************   Miscellaneous  *************************/


function enableDebugger() {
	acrobatDebugger.enabled = true;
}

function quit(code) {	
	jsStack = null;
	curScriptFrameThis = null;
	curScriptFrameScope = null;
	
	clearStack();
	clearInspector();
	
	unsuspend();
	
    resume();
	
	if(code == 1) // Terminate 
		acrobatDebugger.enabled = false;
}

function resume() {
	// resumeCode is defined in C++
	// resumeCode 0  => Progress execution
	// resumeCode -1 => Halt
	resumeCode = 0;
}

function debugEval(text) {
	if (jsStack) {
		var currFrame = jsStack[currentFrameIndex].frame;
		
		var retVal = currFrame.eval(text).return;
		if(typeof retVal  == "object") {
			return retVal.unsafeDereference();
		} else {
			return retVal;
		}
	}
	return undefined;
}

function watch(i, text) {
	if (jsStack && text != "") {
		var ret = debugEval(text);
   		printWatch(i, text, ret.toString());
	} else {
   		printWatch(i, text, IDS_CANNOT_EVALWATCH);
	}
}

function changeLocal(varName, value, bThis) {
	if (jsStack) {
		var ret = debugEval(varName + " = " + value + ";");
		listRecursive(bThis);
	}
}
