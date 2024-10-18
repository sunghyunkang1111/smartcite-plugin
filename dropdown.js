var dialog4 = {
  initialize: function(dialog) {
    var titles = global.uploadedTitles || ["No data available"]; // Default if no data is loaded
    var titleOptions = {};

    // Populate the dropdown with titles
    titles.forEach((title, index) => {
      titleOptions[title] = -(index + 1); // Use negative values as placeholders
    });

    dialog.load({
      subl: titleOptions
    });
  },
  
  subl: function(dialog) {
    console.println("Selection Box Hit");
  },

  getHierChoice: function(e) {
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

  butn: function(dialog) {
    var element = dialog.store().subl;
    var retn = this.getHierChoice(element);
    var ids = global.uploadedIds;

    if (retn) {
      dialog.end("ok");
      console.println("Selected: " + retn.label + " with value: " + retn.value);
      // var sel = ids[retn.value];
      global.dropdownId = ids[retn.value];
      app.alert(ids[retn.value]);

      // try {
      //   var params = {
      //     cVerb: "GET",
      //     cURL: "https://api.smartcite.povio.dev/api/cases/"+sel+"/documents",
      //     aHeaders: [{ name: "x-api-key", value: "aaab07c0-cce0-4014-8045-76a2db8f745a" }]
      //   };
      //   var responseStream = Net.HTTP.request(params);
      //   var response = SOAP.stringFromStream(responseStream);
    
      //   // Parse the JSON and extract titles from the `items` array
      //   var data = JSON.parse(response);
      //   if (Array.isArray(data.items)) {
      //     global.uploadedTitles = data.items.map(item => item.title); // Extract titles from items array
      //     global.uploadedMediaUrl = data.items.map(item => item.mediaUrl);
      //   } else {
      //     app.alert("Data format unexpected: 'items' is not an array.");
      //     global.uploadedTitles = ["No data available"];
      //   }
      //   console.println(response);
      //   console.println("\n\nTitles stored in global: " + global.uploadedTitles);
      // }
      // catch (e) {
      //   app.alert({ cMsg: e.message, cTitle: "Exception" });
      // }



      
      try {
        this.gotoNamedDest("dest" + retn.value);
      } catch (err) {
        console.println("Navigation error: " + err);
      }

      app.alert("You have selected: " + retn.label);
    } else {
      app.alert("Please make a selection, or cancel.");
    }
  },

  cncl: function(dialog) {
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
