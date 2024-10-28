// Function to add a citation
function addCitation() {
    var citationField = this.getField("CitationField"); // Get the citation text field
    if (!citationField) {
        app.alert("Citation field not found. Please ensure it's named 'CitationField'.");
        return;
    }

    var citationText = citationField.value; // Get the text from the citation field
    if (citationText && citationText.length > 0) {
        var pageNum = this.pageNum; // Get the current page number
        var citationRect = [100, 700, 400, 750]; // Position of the citation annotation

        // Create a text annotation for the citation
        this.addAnnot({
            type: "Text",
            page: pageNum,
            rect: citationRect,
            contents: citationText,
            author: "Your Name", // Replace with your name or preferred author
            title: "Citation",
            fillColor: color.white,
            strokeColor: color.black
        });

        app.alert("Citation added successfully!");
    } else {
        app.alert("Please enter citation details in the field.");
    }
}

// Call the function
addCitation();
