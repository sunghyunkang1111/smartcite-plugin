var selectedFile = app.browseForDoc("Select a PDF file to open:");

// Check if a file was selected
if (selectedFile) {
    // Open the selected PDF file
    app.openDoc(selectedFile);
} else {
    app.alert("No file selected.");
}

