// WriteStatus( code, reason = "" )
//
// Writes the HTTP status code with optional reason. Can only be used before any
// WriteHeader(), WriteHtml(), and WriteHtmlRaw() functions. If data has already
// been sent via these calls, then WriteStatus() will return an
// error.
WriteStatus( code, reason := "" );

// WriteHeader( name, value )
//
// Writes a header to the "virtual webpage" that's to be sent
// to the connecting browser. Can only be used before any
// WriteHtml() or WriteHtmlRaw() commands and after optionally WriteStatus(). If data has already
// been sent via these calls, then WriteHeader() will return an
// error.
//
// This function can also be used to change the content type from
// the default "text/html" by writing a Content-Type header name.
WriteHeader( name, value );

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
