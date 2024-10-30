     
    var highlightColor = color.yellow; // Change to your desired highlight color

    // Function to highlight specific texts
    function highlightSpecificTexts() {
        var keywords = global.PDFhighlightText;
        // app.alert(keywords.length);
        // app.alert(keywords[0]);
        var numPages = this.numPages; // Get the total number of pages

        for (var i = 0; i < numPages; i++) { // Loop through each page
            var numWords = this.getPageNumWords(i); // Get the number of words on the page
            var temp = "";
            for (var j = 0; j < numWords; j++) { // Loop through each word
                var word = this.getPageNthWord(i, j, true); // Get the current word

                // Ensure the word is a valid string and not empty
                if (typeof word === "string" && word.length > 0) {
                    // Check if the word matches any of the keywords
                    for (var k = 0; k < keywords.length; k++) {
                        // Check for case-insensitive match
                        // app.alert(keywords[k].toLowerCase());
                        var key = keywords[k].toLowerCase();
                        if(!temp.length) temp = word.toLowerCase();
                        else temp = temp + " " + word.toLowerCase();
                        var fla = j + 1;
                        while(key.substring(0, temp.length) == temp && key.length > temp.length) {
                            // app.alert(temp);
                            // app.alert(key.substring(0, temp.length));
                            temp = temp + " " + this.getPageNthWord(i, fla++, true).toLowerCase();
                        }
                        // app.alert(temp);
                        if (temp == key) {
                            var quads1 = this.getPageNthWordQuads(i, j); // Get the coordinates of the word
                            var quads2 = this.getPageNthWordQuads(i, fla - 1);
                            if (quads1.length > 0) {
                                // app.alert(quads);
                                // Calculate the rectangle using the quads
                                var left = Math.min(quads1[0][0], quads1[0][6]); // x of left side
                                var bottom = Math.min(quads1[0][1], quads1[0][3]); // y of bottom side
                                var right = Math.max(quads2[0][2], quads2[0][4]); // x of right side
                                var top = Math.max(quads2[0][5], quads2[0][7]); // y of top side
                                

                                var highlightRect = [left, bottom, right, top];

                                this.addAnnot({
                                    page: i,
                                    type: "Square",
                                    rect: highlightRect,
                                    fillColor: highlightColor,
                                    strokeColor: highlightColor,
                                    opacity: 0.2,
                                });
                            }
                            
                        }
                        else temp = "";
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




    