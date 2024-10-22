var runExtract1 = app.trustedFunction(function (sel) {
  app.beginPriv();
  try {
    var PDFparams = {
      cVerb: "GET",
      cURL: "https://api.smartcite.povio.dev/api/cases/" + sel + "/documents",
      aHeaders: [{ name: "x-api-key", value: "aaab07c0-cce0-4014-8045-76a2db8f745a" }]
    };
    var PDFresponseStream = Net.HTTP.request(PDFparams);
    var PDFresponse = SOAP.stringFromStream(PDFresponseStream);

    // Parse the JSON and extract titles from the `items` array
    var PDFdata = JSON.parse(PDFresponse);
    if (Array.isArray(PDFdata.items)) {
      global.PDFuploadedTitles = PDFdata.items.map(item => item.title); // Extract titles from items array
      global.PDFuploadedMediaUrl = PDFdata.items.map(item => item.mediaUrl);
    } else {
      app.alert("Data format unexpected: 'items' is not an array.");
      global.PDFuploadedTitles = ["No data available"];
    }
    console.println(PDFresponse);
    console.println("\n\nTitles stored in global: " + global.PDFuploadedTitles);
  }
  catch (e) {
    app.alert({ cMsg: e.message, cTitle: "Exception" });
  }
  app.endPriv();
});


var downloadAndOpenPDF = app.trustedFunction(function (URL) {
  app.beginPriv();
  var pdfURL = URL; // full URL here
  var encodedURL = encodeURIComponent(pdfURL); // Encode the URL
  // Path to your batch file
  var batFile = "C:/Program Files/Adobe/Acrobat DC/Acrobat/Javascripts/launch_pdf.bat";
  // Path to the parameter file
  var paramFilePath = "C:/Program Files/Adobe/Acrobat DC/Acrobat/Javascripts/parameter.pdf";
  // Write the encoded URL to the parameter file
  trustedWriteToFile(paramFilePath, URL);
  // Launch the batch file
  
  app.launchURL("file:///" + batFile);
  app.endPriv();
});


// Trusted function to create a document and write to a file
var trustedWriteToFile = app.trustedFunction(function (filePath, data) {
  app.beginPriv();
  // Create a new blank document (automatically starts with one page)
  var doc = app.newDoc(); // This creates a new document with one page
  // Create a text field on the first page
  var textField = doc.addField("TextField1", "text", 0, [50, 50, 300, 100]); // Adjust coordinates as needed
  textField.value = data; // Set the value of the text field to the encoded URL
  // Debugging alert to check the filePath
  app.alert("Saving document to: " + filePath);
  // Save the document as a PDF
  try {
      doc.saveAs(filePath); // Save without additional options
  } catch (e) {
      app.alert("Error saving document: " + e.message);
  }
  // Close the document after saving
  doc.closeDoc(true);
  app.alert("Saved correctly");
  app.endPriv();
});


var selUri = {};
var dialog4 = {
  initialize: function (dialog) {
    var titles = global.uploadedTitles || ["No data available"]; // Default if no data is loaded
    var titleOptions = {};
    var ids = global.uploadedIds;
    var cnt = 0;

    // Populate the dropdown with titles
    titles.forEach((title, index) => {
      var sectionOptions = {};
      var sel = ids[index];
      runExtract1(sel);
      var PDFTitles = global.PDFuploadedTitles;
      var PDFMediaUrl = global.PDFuploadedMediaUrl;
      PDFTitles.forEach((pdftitle, index) => {
        sectionOptions[pdftitle] = -(cnt + 1);
        selUri[cnt + 1] = PDFMediaUrl[index];
        cnt = cnt + 1;
      })
      titleOptions[title] = sectionOptions;
      // titleOptions[title] = -(index + 1); // Use negative values as placeholders
    });

    dialog.load({
      subl: titleOptions
    });
  },

  subl: function (dialog) {
    console.println("Selection Box Hit");
  },

  getHierChoice: function (e) {
    if (typeof e === "object") {
      for (var i in e) {
        if (typeof e[i] === "object") {
          var retn = this.getHierChoice(e[i]);
          if (retn) {
            retn.label = i + ", " + retn.label;
            return retn;
          }
        } else if (e[i] > 0) {
          return { label: i, value: e[i] };
        }
      }
    }
  },

  butn: function (dialog) {

    var element = dialog.store().subl;
    app.alert(element);
    var retn = this.getHierChoice(element);


    if (retn) {
      dialog.end("ok");
      console.println("Selected: " + retn.label + " with value: " + retn.value + "URl : " + selUri[retn.value]);
      downloadAndOpenPDF(selUri[retn.value]);
    } else {
      app.alert("Please make a selection, or cancel.");
    }
  },

  cncl: function (dialog) {
    dialog.end("cancel");
  },

  description: {
    name: "List of Document",
    elements: [
      {
        type: "view",
        align_children: "align_left",
        elements: [
          {
            type: "cluster",
            name: "Names",
            elements: [
              { type: "static_text", name: "Make a selection" },
              { type: "hier_list_box", item_id: "subl", char_width: 20, height: 200 }
            ]
          },
          {
            type: "view",
            align_children: "align_row",
            elements: [
              { type: "button", item_id: "cncl", name: "Cancel" },
              { item_id: "butn", type: "button", name: "Select" }
            ]
          }
        ]
      }
    ]
  }
};

app.addMenuItem({
  cName: "myDialog",
  cUser: "List of Document",
  cParent: "Edit",
  cExec: "app.execDialog(dialog4)",
  nPos: 0
});
