app.addMenuItem({ 
  cName: "Upload Data", 
  nPos: "Close", 
  cParent: "File", 
  cExec: "runExtract()" 
});

var runExtract = app.trustedFunction(function() {
  app.beginPriv();
  try {
    var params = {
      cVerb: "GET",
      cURL: "https://api.smartcite.povio.dev/api/cases",
      aHeaders: [{ name: "x-api-key", value: "aaab07c0-cce0-4014-8045-76a2db8f745a" }]
    };
    var responseStream = Net.HTTP.request(params);
    var response = SOAP.stringFromStream(responseStream);

    // Parse the JSON and extract titles from the `items` array
    var data = JSON.parse(response);
    if (Array.isArray(data.items)) {
      global.uploadedTitles = data.items.map(item => item.title); // Extract titles from items array
      global.uploadedIds = data.items.map(item => item.id);
    } else {
      app.alert("Data format unexpected: 'items' is not an array.");
      global.uploadedTitles = ["No data available"];
    }
    console.println(response);
    console.println("\n\nTitles stored in global: " + global.uploadedTitles);
  }
  catch (e) {
    app.alert({ cMsg: e.message, cTitle: "Exception" });
  }
  app.endPriv();
});


