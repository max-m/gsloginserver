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
	
    try
    {	
        if( !FileExists(fso, hostspath) )
            fso.CreateTextFile( hostspath );
        else
        {
            var hosts_file = fso.GetFile( hostspath );
            gAttrs = hosts_file.attributes
            hosts_file.attributes = 0;
        }
        
        CleanUpHosts( fso, shell );
				
        var hostsObj = fso.GetFile( hostspath );
        var hosts = hostsObj.OpenAsTextStream( 8, 0 );
		
        hosts.WriteLine( "" ); // empty line - just to play it safe
        hosts.WriteLine( ip + " bf2web.gamespy.com #bf2_statistics_line" );
        hosts.WriteLine( ip + " gpcm.gamespy.com #bf2_statistics_line" );
        hosts.WriteLine( ip + " gpsp.gamespy.com #bf2_statistics_line" );
        hosts.WriteLine( ip + " battlefield2.available.gamespy.com #bf2_statistics_line" );
		
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
			
    try
    {
        var file = fso.OpenTextFile( hostspath, 1, true );
        var content = "";
            
        while( !file.AtEndOfStream )
        {
            var line = file.ReadLine( );
                
            if( line.indexOf("#bf2_statistics_line") == -1 && line.length > 0 )
                content += line + "\n";
        }
        
        file.close( );
        
        file = fso.OpenTextFile( hostspath, 2 );
        file.WriteLine( content );
        file.close( );
        
        fso.GetFile(hostspath).attributes = gAttrs;
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