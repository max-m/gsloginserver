<html>
	<head>
		<title> BF2 Starter </title>
		
		<?php
			if( isset($_SERVER['HTTP_USER_AGENT']) && strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE') !== false )
				echo( '
			<script type="text/javascript">
				<!--		
					var gScriptDisabled = true;
					window.onbeforeunload = CleanUpHosts;
					
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
								bf2_reg = "HKEY_LOCAL_MACHINE\\\\SOFTWARE\\\\Electronic Arts\\\\EA Games\\\\Battlefield 2\\\\InstallDir";
							else
								bf2_reg = "HKEY_LOCAL_MACHINE\\\\SOFTWARE\\\\Wow6432Node\\\\Electronic Arts\\\\EA GAMES\\\\Battlefield 2\\\\InstallDir";
							
							bf2_dir = shell.RegRead( bf2_reg );
						}
						catch( exception ) 
						{
							throw "Battlefield 2 not installed!";
						}

						if( !FileExists(fso, bf2_dir + "\\\\BF2.exe") )
							throw "BF2.exe not found! Check your Registry!";

						return bf2_dir;
					}
					
					function SetHostsFile( fso, shell )
					{
						var env = shell.Environment( "PROCESS" );
						var hostspath = env( "SYSTEMROOT" ) + "\\\\System32\\\\drivers\\\\etc\\\\hosts.ics";
						
						if( !FileExists(fso, hostspath) )
							fso.CreateTextFile( hostspath );
						else
						{	
							if( FileExists(fso, hostspath + ".backup") )
							{
								fso.DeleteFile( hostspath );
								fso.CopyFile( hostspath + ".backup", hostspath );
							}
							else
								fso.CopyFile( hostspath, hostspath + ".backup" );
						}
						
						var hostsObj = fso.GetFile( hostspath );
						var hosts = hostsObj.OpenAsTextStream( 8, 0 );
						hosts.WriteLine( "'.$_SERVER['REMOTE_ADDR'].' bf2web.gamespy.com" );
						hosts.WriteLine( "'.$_SERVER['REMOTE_ADDR'].' gpcm.gamespy.com" );
						hosts.WriteLine( "'.$_SERVER['REMOTE_ADDR'].' gpsp.gamespy.com" );
						hosts.WriteLine( "'.$_SERVER['REMOTE_ADDR'].' battlefield2.available.gamespy.com" );
						hosts.close();
					}
					
					function CleanUpHosts( )
					{
						if( gScriptDisabled )
							return;
						
						var fso, shell;
						
						try
						{
							fso = new ActiveXObject( "Scripting.FileSystemObject" );
							shell = new ActiveXObject( "WScript.Shell" );
						}
						catch( exception )
						{
							return;
						}
						
						var env = shell.Environment( "PROCESS" );
						var hostspath = env( "SYSTEMROOT" ) + "\\\\System32\\\\drivers\\\\etc\\\\hosts.ics";
						
						if( FileExists(fso, hostspath + ".backup") )
						{
							fso.DeleteFile( hostspath );
							fso.CopyFile( hostspath + ".backup", hostspath );
							fso.DeleteFile( hostspath + ".backup" );
						}
					}

					function StartBF2( ) 
					{
						var fso, shell;
						
						try
						{
							fso = new ActiveXObject( "Scripting.FileSystemObject" );
							shell = new ActiveXObject( "WScript.Shell" );
							gScriptDisabled = false;
						}
						catch( exception )
						{
							alert( "You have to activate\n[Initialize and script Activex controls not marked as safe for script]\nin Tools -> Internet Options -> Security!" );
							return;
						}
						
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
							return;
						}
						
						SetHostsFile( fso, shell );
						
						shell.CurrentDirectory = bf2_dir;
						shell.Run( "\\"" + bf2_dir + "\\\\BF2.exe\\"" );
						
						delete fso;
						delete shell;
						return;
					}					
				//-->
			</script>' );
		?>
		
	</head>

	<body>
		<h1> Battlefield 2 Starter </h1>
		
		<p>
			With this site you can start Battlefield 2 and prepear your 	<br />
			computer for working with the Battlefield 2 Statistics system. 	<br />
			Therefore you have to open this site with the Internet Explorer -
		<?php
			if( isset($_SERVER['HTTP_USER_AGENT']) && strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE') !== false )
				echo( "\twhat you did!										<br /> <br /> <br />
			<BLOCKQUOTE>
				Click here to start Battlefield 2: 							<br />
				<button onclick=\"StartBF2();\"> Start Battlefield 2 </button>	
			</BLOCKQUOTE>															<br />
			<pre>
Clicking on the above button will append several entries to your 
hosts.ics which will enable the Battlefield 2 Statistics system.
They will be deleted when you exit this site.</pre>
			Should you have any security scruples, just study the source! 	<br />
			
																			<br />
																			<br />" );
			else 
				echo( "\twhat you didn't do! 						<br /> <br />
			Please open this site with the Internet Explorer: 		<br />
			<table valign=\"top\" border=0> <td>Press </td> <td><img src=\"Win-Key.jpg\" alt=\"[WINDOWS-KEY]\"></td> <td> + </td> <td><img src=\"R-Key.jpg\" alt=\"[R]\"></td> <td> and enter this:</td></table> 
			<pre> iexplore.exe http://{$_SERVER["SERVER_NAME"]}{$_SERVER["REQUEST_URI"]} </pre>" );
		?>	
																			<br />
			Alternatively you can also download this file, 					<br />
			which will do the same: <a href="BF2_Starter.js">BF2_Starter.js</a>		
		</p>		

	</body>
</html>