// Trig Functions
// Sin, Cos, Tan take arguments in Radians
// ASin, ACos, ATan return results in Radians
Sin( x );
Cos( x );
Tan( x );
ASin( x );
ACos( x );
ATan( x );

// Radian <-> Degrees Conversion
RadToDeg( radians );
DegToRad( degrees );

Min( x, y := 0 ); // returns minumum of x and y or lowest Element in given array
Max( x, y := 0 ); // returns maximum of x and y or biggest Element in given array
Pow( x, y ); // returns x ^ y
Sqrt( x ); // returns square root of x
Root( x, y ); // returns y root of x (same as Pow(x,1.0/y))
Abs( x ); // returns absolute value (works on Reals and Integers)
Log10( x ); // Returns log(base 10) of x
LogE( x ); // Returns natural log of x

// rounding functions
Ceil( x ); // returns Real that is smallest integer >= x
Floor( x ); // returns Real that is largest integer <= x

// functions to return some constants
ConstPi();
ConstE();

// I'm just playing with this one...
FormatRealToString( value, precision );
