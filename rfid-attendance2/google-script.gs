//?data1=000&data2=Ahmad&data3=Logs&data4=Saad&data5=25-01-1970&data6=+9234582459876
var ss = SpreadsheetApp.openById('ENTER_SHEET_ID');
var sheet = ss.getSheetByName('ENTER_SHEET_NAME');
var timezone = "asia/karachi"
var hours = 0

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
function doGet(e){
  Logger.log( JSON.stringify(e) );
  //----------------------------------------------------------------------------------
  //write_google_sheet() function in esp32 sketch, is send data to this code block
  //----------------------------------------------------------------------------------
  //get gps data from ESP32
  if (e.parameter == 'undefined') {
    return ContentService.createTextOutput("Received data is undefined");
  }
  //----------------------------------------------------------------------------------
  var Curr_Date =new Date(new Date().setHours(new Date().getHours() + hours));
  var Curr_Time     = Utilities.formatDate(Curr_Date, timezone, 'HH:mm:ss');
  var student_id    = stripQuotes(e.parameters.data1);
  var first_name    = stripQuotes(e.parameters.data2);
  var last_name     = stripQuotes(e.parameters.data3);
  var father_name   = stripQuotes(e.parameters.data4);
  var date_of_birth = stripQuotes(e.parameters.data5);
  var phone_number  = stripQuotes(e.parameters.data6);
  //Logger.log('name=' + student_id);
  //----------------------------------------------------------------------------------
  var nextRow = sheet.getLastRow() + 1;
  sheet.getRange("A" + nextRow).setValue(Curr_Date);
  sheet.getRange("B" + nextRow).setValue(Curr_Time);
  sheet.getRange("C" + nextRow).setValue(student_id);
  sheet.getRange("D" + nextRow).setValue(first_name);
  sheet.getRange("E" + nextRow).setValue(last_name);
  sheet.getRange("F" + nextRow).setValue(father_name);
  sheet.getRange("G" + nextRow).setValue(date_of_birth);
  sheet.getRange("H" + nextRow).setValue(phone_number);
  //----------------------------------------------------------------------------------

  //returns response back to ESP32
  return ContentService.createTextOutput("Data is stored in Google Sheet.");
  //----------------------------------------------------------------------------------
}


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
function stripQuotes( value ) {
  return value.toString().replace(/^["']|['"]$/g, "");
}

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//Extra Function. Not used in this project.
//planning to use in future projects.
//this function is used to handle POST request
function doPost(e) {
  var val = e.parameter.value;
  
  if (e.parameter.value !== undefined){
    var range = sheet.getRange('A2');
    range.setValue(val);
  }
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM