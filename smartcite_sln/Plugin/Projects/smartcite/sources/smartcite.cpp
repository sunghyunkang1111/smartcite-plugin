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

ASFixedRect annotBounds;

ASBool MyWordCallback(PDWordFinder wObj, PDWord wInfo, ASInt32 pgNum, void* clientData) {
    std::vector<docWordsFinderData>* docsData = (std::vector<docWordsFinderData>*)clientData;

    static std::vector<std::string> wordBufferList; // List of words in the current sentence
    static std::vector<ASFixedQuad> quadList;       // List of quads for each word in the sentence
    char wordBuffer[256] = { 0 };

    // Extract the current word
    PDWordGetString(wInfo, wordBuffer, sizeof(wordBuffer));
    std::string currentWord = wordBuffer;

    // Get bounding quad of the current word
    ASFixedQuad wordQuad;
    PDWordGetNthQuad(wInfo, 0, &wordQuad);

    // Add current word and quad to buffers
    wordBufferList.push_back(currentWord);
    quadList.push_back(wordQuad);

    // Check for sentence-ending punctuation
    bool isSentenceEnd = (currentWord.back() == '.' || currentWord.back() == '!' || currentWord.back() == '?');

    // If end of sentence or segment
    if (isSentenceEnd) {
        for (int i = 0; i < docsData->size(); i++) {
            for (int j = 0; j < docsData->at(i).processedData->citations.size(); j++) {
                const std::string& citation = docsData->at(i).processedData->citations[j];

                // Rebuild sentence from word buffer to locate citation
                std::string sentenceBuffer;
                for (const auto& word : wordBufferList) {
                    if (!sentenceBuffer.empty()) sentenceBuffer += " ";
                    sentenceBuffer += word;
                }

                // Find citation within the sentence
                size_t pos = sentenceBuffer.find(citation);
                if (pos != std::string::npos) {
                    finderOutputData finderData = { pgNum, citation, docsData->at(i).processedData->docInfo };
                    docsData->at(i).outputData.push_back(finderData);
                    // Highlight only the matching words
                    size_t startWordIndex = 0;
                    size_t charCount = 0;

                    for (size_t k = 0; k < wordBufferList.size(); ++k) {
                        const std::string& word = wordBufferList[k];
                        size_t wordLen = word.length();

                        // Check if the current word is part of the citation match
                        if (charCount >= pos && charCount < pos + citation.length()) {
                            // Add highlight for this word
                            AVDoc avDoc = AVAppGetActiveDoc();
                            if (avDoc) {
                                PDDoc pdDoc = AVDocGetPDDoc(avDoc);
                                PDPage pdPage = PDDocAcquirePage(pdDoc, pgNum);
                                if (pdPage) {
                                    // Add annotation for the word
                                    ASFixedRect annotBounds;
                                    annotBounds.left = quadList[k].bl.h;
                                    annotBounds.bottom = quadList[k].bl.v;
                                    annotBounds.right = quadList[k].tr.h;
                                    annotBounds.top = quadList[k].tr.v;

                                    PDAnnot pdAnnot = PDPageAddNewAnnot(pdPage, -1, ASAtomFromString("Highlight"), &annotBounds);
                                    CosObj annotObj = PDAnnotGetCosObj(pdAnnot);

                                    // Add QuadPoints for this word
                                    CosDoc cosDoc = PDDocGetCosDoc(pdDoc);
                                    CosObj quadArray = CosNewArray(cosDoc, false, 8);
                                    CosArrayPut(quadArray, 0, CosNewFixed(cosDoc, false, quadList[k].tl.h));
                                    CosArrayPut(quadArray, 1, CosNewFixed(cosDoc, false, quadList[k].tl.v));
                                    CosArrayPut(quadArray, 2, CosNewFixed(cosDoc, false, quadList[k].tr.h));
                                    CosArrayPut(quadArray, 3, CosNewFixed(cosDoc, false, quadList[k].tr.v));
                                    CosArrayPut(quadArray, 4, CosNewFixed(cosDoc, false, quadList[k].bl.h));
                                    CosArrayPut(quadArray, 5, CosNewFixed(cosDoc, false, quadList[k].bl.v));
                                    CosArrayPut(quadArray, 6, CosNewFixed(cosDoc, false, quadList[k].br.h));
                                    CosArrayPut(quadArray, 7, CosNewFixed(cosDoc, false, quadList[k].br.v));
                                    CosDictPutKeyString(annotObj, "QuadPoints", quadArray);

                                    // Set annotation color
                                    PDColorValueRec color;
                                    color.space = PDDeviceRGB;
                                    color.value[0] = ASFloatToFixed(1.0f);
                                    color.value[1] = ASFloatToFixed(1.0f);
                                    color.value[2] = ASFloatToFixed(0.0f);
                                    PDAnnotSetColor(pdAnnot, &color);

                                    AVPageView pageView = AVDocGetPageView(avDoc);
                                    AVPageViewDrawNow(pageView);
                                    PDPageRelease(pdPage);
                                }
                            }
                        }

                        // Update character count
                        charCount += wordLen + 1; // Account for space
                    }

                    std::cout << "Highlighted citation: '" << citation << "' on page " << pgNum + 1 << std::endl;
                }
            }
        }

        // Reset buffers
        wordBufferList.clear();
        quadList.clear();
    }

    return true; // Continue enumeration
}

void ExtractWordsFromPDF(std::vector<docWordsFinderData>& docsData) {
    // Encoding info and vector: Pass NULL for defaults
    ASUns16* outEncInfo = NULL;
    char** outEncVec = NULL;

    // Ligature table: Use default ligatures by passing NULL
    char** ligatureTbl = NULL;

    // Algorithm version: Use the latest
    ASInt16 algVersion = WF_LATEST_VERSION;

    // Word-finding options: Ignored in Acrobat 5.0+, pass 0
    ASUns16 rdFlags = 0;

    // Client data: Pass NULL if no custom data is needed
    void* clientData = NULL;

    AVDoc avDoc = AVAppGetActiveDoc();
    if (!avDoc) {
        AVAlertNote("No active document found!");
        return;
    }

    PDDoc pdDoc = AVDocGetPDDoc(avDoc);
    if (!pdDoc) {
        AVAlertNote("No valid PDF document found!");
        return;
    }

    // Create Word Finder
    ASBool success = false;
    PDWordFinder wordFinder = PDDocCreateWordFinder(pdDoc, outEncInfo, outEncVec, ligatureTbl, algVersion, rdFlags, clientData);
    if (wordFinder) {
        // Use the Word Finder to enumerate or acquire words
        // Example: Enumerate words on the first page
        PDWordFinderEnumWords(wordFinder, 0, MyWordCallback, (void*)&docsData);

        // Destroy the Word Finder
        PDWordFinderDestroy(wordFinder);
    }
    else {
        std::cerr << "Failed to create Word Finder." << std::endl;
    }
}

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
    // Initialize cURL
    curl_global_init(CURL_GLOBAL_ALL);

    // Get document data and process
    std::vector<docProcessedData> output = getDocumentData();

    std::vector<docWordsFinderData> dataToProcess;

    for (int i = 0; i < output.size(); i++)
    {
        docWordsFinderData dataToFind;
        dataToFind.processedData = &output[i];
        dataToProcess.push_back(dataToFind);
    }

    ExtractWordsFromPDF(dataToProcess);

    for (auto& data : dataToProcess)
    {
        for (auto& info : data.outputData)
        {
            downloadUrl(info.docInfo.mediaUrl, info.docInfo.filename);
            openFileUrl(info.docInfo.filename);
        }
    }

    // Clean up
    curl_global_cleanup();
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