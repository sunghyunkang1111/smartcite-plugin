/*********************************************************************

 ADOBE SYSTEMS INCORPORATED
 Copyright (C) 1998-2006 Adobe Systems Incorporated
 All rights reserved.

 NOTICE: Adobe permits you to use, modify, and distribute this file
 in accordance with the terms of the Adobe license agreement
 accompanying it. If you have received this file from a source other
 than Adobe, then your use, modification, or distribution of it
 requires the prior written permission of Adobe.

 -------------------------------------------------------------------*/
/** 
\file BasicPlugin.cpp

  - This file implements the functionality of the BasicPlugin.
*********************************************************************/


// Acrobat Headers.
#ifndef MAC_PLATFORM
#include "PIHeaders.h"
#include "apiRequests.h"
#include <iostream>
#endif

#define INITIAL_STR_SIZE 1024
const unsigned int MAX_BUFFER_LENGTH = 2048;

/*-------------------------------------------------------
	Constants/Declarations
-------------------------------------------------------*/
// This plug-in's name, you should specify your own unique name here.
#pragma  message ("Please specify your own UNIQUE plug-in name. Remove this message if you have already done so")
const char* MyPluginExtensionName = "ADBE:BasicPlugin";

/* A convenient function to add a menu item for your plugin.
*/
ACCB1 ASBool ACCB2 PluginMenuItem(char* MyMenuItemTitle, char* MyMenuItemName);

/*-------------------------------------------------------
	Functions
r-------------------------------------------------------*/

/* MyPluginSetmenu
** ------------------------------------------------------
**
** Function to set up menu for the plugin.
** It calls a convenient function PluginMenuItem.
** Return true if successful, false if failed.
*/
ACCB1 ASBool ACCB2 MyPluginSetmenu()
{
	// Add a new menu item under Acrobat SDK submenu.
	// The new menu item name is "ADBE:BasicPluginMenu", title is "Basic Plugin".
	// Of course, you can change it to your own.
	return PluginMenuItem("Basic Plugin", "ADBE:BasicPluginMenu"); 
}
	
//void createFile()
//{
//    // File path for writing "Hello"
//    const char* filePath = "C:/Users/ddd/Desktop/test.txt";
//    FILE* file = fopen(filePath, "w");
//    if (file) {
//        fprintf(file, "Hello");
//        fclose(file);
//    }
//}

void ExtractArtifacts(const std::string& strContents, std::vector<std::string>& vArtifact)
{
	std::string::size_type nPos = 0, nPrevPos = 0;

	// Parse through the "Contents" stream
	while ((nPos = strContents.find('\n', nPos)) != std::string::npos) {
		std::string strTmp = strContents.substr(nPrevPos, nPos - nPrevPos);

		if (strTmp.length() > 9 && !strTmp.substr(0, 9).compare("/Artifact")) {
			vArtifact.push_back(strTmp);
		}

		// Move on to the next line
		nPrevPos = ++nPos;
	}
}

ASFixedRect annotBounds;

ASBool MyWordCallback(PDWordFinder wObj, PDWord wInfo, ASInt32 pgNum, void* clientData) {
    char wordBuffer[256] = { 0 };

    // Extract the string representation of the word
    PDWordGetString(wInfo, wordBuffer, sizeof(wordBuffer));

    // Check if the word matches "Baystream"
    if (strcmp(wordBuffer, "Baystream") == 0) {
        // Get the bounding rectangle (quad) of the word
        ASFixedQuad wordQuad;
        PDWordGetNthQuad(wInfo, 0, &wordQuad);

        // Prepare annotation bounds
        ASFixedRect annotBounds;
        annotBounds.left = wordQuad.bl.h;
        annotBounds.bottom = wordQuad.bl.v;
        annotBounds.right = wordQuad.tr.h;
        annotBounds.top = wordQuad.tr.v;

        // Get the current document and page
        AVDoc avDoc = AVAppGetActiveDoc();
        if (avDoc) {
            PDDoc pdDoc = AVDocGetPDDoc(avDoc);
            PDPage pdPage = PDDocAcquirePage(pdDoc, pgNum);
            PDPageGetCropBox(pdPage, &annotBounds);

            if (pdPage) {
                // Add highlight annotation
                PDAnnot pdAnnot = PDPageAddNewAnnot(pdPage, -1, ASAtomFromString("Highlight"), &annotBounds);

                // Set annotation color (e.g., yellow)
                PDColorValueRec color;
                color.space = PDDeviceRGB;
                color.value[0] = ASFloatToFixed(1.0f); // Red
                color.value[1] = ASFloatToFixed(1.0f); // Green
                color.value[2] = ASFloatToFixed(0.0f); // Blue
                PDAnnotSetColor(pdAnnot, &color);

                // Set annotation flags
                PDAnnotSetFlags(pdAnnot, pdAnnotPrint);

                PDTextAnnotSetOpen(pdAnnot, true);

                // Force a redraw of the page
                AVPageView pageView = AVDocGetPageView(avDoc);
                AVPageViewDrawNow(pageView);

                // Release the page
                PDPageRelease(pdPage);

                std::cout << "Added highlight annotation for 'Baystream' on page " << pgNum + 1 << "." << std::endl;
            }
        }
    }

    return true; // Continue enumeration
}

//void ExtractWordsFromPDF() {
//    // Encoding info and vector: Pass NULL for defaults
//    ASUns16* outEncInfo = NULL;
//    char** outEncVec = NULL;
//
//    // Ligature table: Use default ligatures by passing NULL
//    char** ligatureTbl = NULL;
//
//    // Algorithm version: Use the latest
//    ASInt16 algVersion = WF_LATEST_VERSION;
//
//    // Word-finding options: Ignored in Acrobat 5.0+, pass 0
//    ASUns16 rdFlags = 0;
//
//    // Client data: Pass NULL if no custom data is needed
//    void* clientData = NULL;
//
//    AVDoc avDoc = AVAppGetActiveDoc();
//    if (!avDoc) {
//        AVAlertNote("No active document found!");
//        return;
//    }
//
//    PDDoc pdDoc = AVDocGetPDDoc(avDoc);
//    if (!pdDoc) {
//        AVAlertNote("No valid PDF document found!");
//        return;
//    }
//
//    // Create Word Finder
//    ASBool success = false;
//    PDWordFinder wordFinder = PDDocCreateWordFinder(pdDoc, outEncInfo, outEncVec, ligatureTbl, algVersion, rdFlags, clientData);
//    if (wordFinder) {
//        // Use the Word Finder to enumerate or acquire words
//        // Example: Enumerate words on the first page
//        PDWordFinderEnumWords(wordFinder, 0, MyWordCallback, NULL);
//
//        // Destroy the Word Finder
//        PDWordFinderDestroy(wordFinder);
//    }
//    else {
//        std::cerr << "Failed to create Word Finder." << std::endl;
//    }
//}

/**		BasicPlugin project is an Acrobat plugin sample with the minimum code 
	to provide an environment for plugin developers to get started quickly.
	It can help Acrobat APIs' code testing, too.  
		This file implements the functionality of the BasicPlugin. It adds a 
	new menu item that will show a message of some simple information about 
	the plugin and front PDF document. Users can modify and add code in this 
	file only to make a simple plugin of their own.   
		
		  MyPluginCommand is the function to be called when executing a menu.
	This is the entry point for user's code, just add your code inside.

	@see ASExtensionGetRegisteredName
	@see AVAppGetActiveDoc
	@see PDDocGetNumPages
*/ 
ACCB1 void ACCB2 MyPluginCommand(void* clientData) {
    //ExtractWordsFromPDF();
    size_t strSize = INITIAL_STR_SIZE;
    char* str = (char*)malloc(strSize);
    if (!str) {
        fprintf(stderr, "Failed to allocate memory for message buffer\n");
        return;
    }
    str[0] = '\0';

    // Plugin initialization message
    ASAtom NameAtom = ASExtensionGetRegisteredName(gExtensionID);
    const char* name = ASAtomGetString(NameAtom);
    snprintf(str, strSize, "This menu item is added by plugin %s.\n", name);

    // Initialize cURL
    curl_global_init(CURL_GLOBAL_ALL);

    // Get document data and process
    std::vector<std::pair<std::string, std::vector<std::string>>> output = getDocumentData();

    // Clean up
    curl_global_cleanup();

    AVAlertNote(str);
    free(str);
}

/* MyPluginIsEnabled
** ------------------------------------------------------
** Function to control if a menu item should be enabled.
** Return true to enable it, false not to enable it.
*/
ACCB1 ASBool ACCB2 MyPluginIsEnabled(void *clientData)
{
	// always enabled.
	return true;
	
	// this code make it is enabled only if there is a open PDF document. 
	/* return (AVAppGetActiveDoc() != NULL); */
}

///Callback proc for mode switch notification.
ACCB1 void ACCB2 AcroAppModeSwitchNotification(void* clientData)
{
	AVAlertNote("Acrobat SDK - Mode Switch Notification");
}