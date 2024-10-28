// Ensure this script runs in the correct context
var textField = this.getField("TextField"); // Assuming you named your text field "TextField"

if (textField) {
    var selectedText = textField.value; // Get the text from the field
    
    if (selectedText && selectedText.length > 0) {
        var highlightColor = color.yellow; // Define highlight color
        var rect = textField.rect; // Get the rectangle of the text field

        // Add the highlight annotation
        this.addAnnot({
            type: "Highlight",
            page: this.pageNum,
            rect: rect,
            strokeColor: highlightColor,
            fillColor: highlightColor
        });
        app.alert("Text highlighted successfully!");
    } else {
        app.alert("Please enter some text in the field to highlight.");
    }
} else {
    app.alert("Text field not found. Please ensure it's named 'TextField'.");
}
