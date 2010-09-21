try
{
    WScript.arguments;
    main( );
}
catch( exception )
{
    // this program was started by a website
}

var gAttrs;

function FileExists( fso, file ) 
{
    if( fso.FileExists(file) )
        return true;

    return false;
}

function GetBF2Dir( fso, shell ) 
{
    var bf2_dir = "";

    try 
    {
        var env = shell.Environment( "System" );
        var bf2_reg;

        if( env("PROCESSOR_ARCHITECTURE") == "x86" )
            bf2_reg = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Battlefield 2\\InstallDir";
        else
            bf2_reg = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Electronic Arts\\EA GAMES\\Battlefield 2\\InstallDir";
							
        bf2_dir = shell.RegRead( bf2_reg );
    }
    catch( exception ) 
    {
        throw "Battlefield 2 not installed!";
    }

    if( !FileExists(fso, bf2_dir + "\\BF2.exe") )
        throw "BF2.exe not found! Check your Registry!";

    return bf2_dir;
}

function SetHostsFile( fso, shell, ip )
{
    var ip = ( ip == null ) ? "127.0.0.1" : ip;
    var env = shell.Environment( "PROCESS" );
    var hostspath = env( "SYSTEMROOT" ) + "\\System32\\drivers\\etc\\hosts.ics";
    var hostspath_backup = hostspath + ".backup";
	
	try
	{	
		if( !FileExists(fso, hostspath) )
			fso.CreateTextFile( hostspath );
		else
		{
			var hosts_file = fso.GetFile( hostspath );
			gAttrs = hosts_file.attributes
			hosts_file.attributes = 0;
			
			if( FileExists(fso, hostspath_backup) )
			{
				var hosts_file_backup = fso.GetFile( hostspath_backup );
				hosts_file_backup.attributes = 0;
				
				fso.DeleteFile( hostspath );
				fso.CopyFile( hostspath_backup, hostspath );
				// delete the old hosts.ics file to renew the entries
			}
			else
				fso.CopyFile( hostspath, hostspath_backup );
		}
				
		var hostsObj = fso.GetFile( hostspath );
		var hosts = hostsObj.OpenAsTextStream( 8, 0 );
		
		hosts.WriteLine( "" ); // empty line - just to play it safe
		hosts.WriteLine( ip + " bf2web.gamespy.com" );
		hosts.WriteLine( ip + " gpcm.gamespy.com" );
		hosts.WriteLine( ip + " gpsp.gamespy.com" );
		hosts.WriteLine( ip + " battlefield2.available.gamespy.com" );
		
		hosts.close( );
	}
	catch( exception )
	{
		throw "Insufficient rights!";
	}
}

function CleanUpHosts( fso, shell )
{
    var env = shell.Environment( "PROCESS" );
    var hostspath = env( "SYSTEMROOT" ) + "\\System32\\drivers\\etc\\hosts.ics";
    var hostspath_backup = hostspath + ".backup";
			
	try
	{
		if( FileExists(fso, hostspath_backup) )
		{
			fso.DeleteFile( hostspath );
			fso.CopyFile( hostspath_backup, hostspath );			
			fso.DeleteFile( hostspath_backup );
			
			// give back hosts.ics its old attributes
			var hosts_file = fso.GetFile( hostspath );
			hosts_file.attributes = gAttrs;
		}
	}
	catch( exception )
	{
		throw "Insufficient rights!";
	}
}

function StartBF2( ) 
{
    var fso = new ActiveXObject( "Scripting.FileSystemObject" );
    var shell = new ActiveXObject( "WScript.Shell" );
	
    var bf2_dir;

    try 
    {
        bf2_dir = GetBF2Dir( fso, shell );
    }
    catch( exception ) 
    {
        shell.PopUp( exception );
				
        delete fso;
        delete shell;
        return -1;
    }
		
	try
	{
		SetHostsFile( fso, shell );
		
		shell.CurrentDirectory = bf2_dir;
		shell.Run( "\"" + bf2_dir + "\\BF2.exe\"", 1, true );
		
		CleanUpHosts( fso, shell );
	}
	catch( exception )
	{
		shell.PopUp( exception );
	}
	
    delete fso;
    delete shell;
    return 0;
}

function main( )
{
    return StartBF2( );
}