if ( WScript.Arguments.Count() < 2 )
{
	WScript.Quit(1);
}

var fs = WScript.CreateObject("Scripting.FileSystemObject");
var env = WScript.CreateObject( "WScript.Shell" ).Environment( "User" )

var source = WScript.Arguments.Item(0);
var target = env("SOURCEMODS") + "\\" + WScript.Arguments.Item(1);

if ( fs.FileExists(source) )
{
	if ( fs.FileExists(target) ) { fs.DeleteFile( target, true ); }
	fs.CopyFile( source, target );
}

WScript.Quit(0);