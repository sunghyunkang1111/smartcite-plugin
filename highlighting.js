var highlightColor = color.yellow; // Change to your desired highlight color

// Function to highlight specific texts
function highlightSpecificTexts() {
    var keywords = global.PDFhighlightText;
    app.alert(keywords.length);
    app.alert(keywords[0]);
    var numPages = this.numPages; // Get the total number of pages

    for (var i = 0; i < numPages; i++) { // Loop through each page
        var numWords = this.getPageNumWords(i); // Get the number of words on the page

        for (var j = 0; j < numWords; j++) { // Loop through each word
            var word = this.getPageNthWord(i, j, true); // Get the current word

            // Ensure the word is a valid string and not empty
            if (typeof word === "string" && word.length > 0) {
                // Check if the word matches any of the keywords
                for (var k = 0; k < keywords.length; k++) {
                    // Check for case-insensitive match
                    if (word.toLowerCase() === keywords[k].toLowerCase()) {
                        var quads = this.getPageNthWordQuads(i, j); // Get the coordinates of the word
                        if (quads.length > 0) {
                            // app.alert(quads);
                            // Calculate the rectangle using the quads
                            var left = Math.min(quads[0][0], quads[0][6]); // x of left side
                            var bottom = Math.min(quads[0][1], quads[0][3]); // y of bottom side
                            var right = Math.max(quads[0][2], quads[0][4]); // x of right side
                            var top = Math.max(quads[0][5], quads[0][7]); // y of top side
                            

                            var highlightRect = [left, bottom, right, top];
                            // app.alert(highlightRect);
                            // Add the highlight annotation
                            this.addAnnot({
                                // page: i,
                                // type: "Highlight",
                                // rect: highlightRect,
                                // fillColor: highlightColor,
                                // strokeColor: highlightColor
                                page: i,
                                type: "Highlight",
                                rect: highlightRect,
                                name: "OnMarketShare",
                                author: "A. C. Robat",
                            });
                            // this.addAnnot({
                            //     type: "Highlight",
                            //     rect: highlightRect,
                            //     fillColor: color.yellow, // Highlight color
                            //     strokeColor: color.yellow // Outline color
                            // });
                            
                        }
                    }
                }
            }
        }
    }
}

// Add the menu item to the Edit menu
app.addMenuItem({
    cName: "HighlightSpecificTextMenuItem",
    cUser: "Highlight Specific Texts",
    cParent: "Edit",
    cExec: "highlightSpecificTexts();",
    nPos: 0 // Position in the menu (0 for top)
});
