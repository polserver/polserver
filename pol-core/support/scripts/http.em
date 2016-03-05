// WriteHtml( html )
//
// Writes a line of HTML to the "virtual webpage" that's to be sent
// to the connecting browser and appends a new-line.
//
WriteHtml( html );

// WriteHtmlRaw( html )
//
// Writes a line of HTML to the "virtual webpage" that's to be sent
// to the connecting browser, but does NOT append a new-line.
//
WriteHtmlRaw( html ); // don't append a newline

// QueryParam( param )
//
// Query's the accessed URL for any embedded parameters.
// (see your favorite HTML docs for parameter embedding! :) )
//
// Example: http://pol.server.com:8080/querytest.ecl?par=wibble&name=
//
// QueryParam( "par" ) ---> "wibble"
// QueryParam( "name" ) --> ""
// QueryParam( "heat" ) --> 0 [integer]
//
QueryParam( param );

// QueryIP()
//
// returns the IP address of the connecting browser
//
// QueryIP() --> "192.168.0.5"
//
QueryIP();