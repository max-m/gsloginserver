<html>
    <head>
    
        <title> BF2 Starter </title>		
        <?php
            if( isset($_SERVER['HTTP_USER_AGENT']) && strpos($_SERVER['HTTP_USER_AGENT'], 'MSI') !== false )
                echo( '    <script text="text/JScript" src="BF2_Starter.js"></script><script language="JScript">
            <!--   
                var gHostsSet = false;
                
                function OnExit( )
                {
                    if( gHostsSet )
                    {
                        var fso = new ActiveXObject( "Scripting.FileSystemObject" );
                        var shell = new ActiveXObject( "WScript.Shell" );
                            
                        CleanUpHosts( fso, shell );
                        gHostsSet = false;

                        delete fso;
                        delete shell;
                    }
                }
                
                function StartBF2_WEB( ) 
                {
                    var fso, shell;
					
                    try
                    {
                        fso = new ActiveXObject( "Scripting.FileSystemObject" );
                        shell = new ActiveXObject( "WScript.Shell" );
                    }
                    catch( exception )
                    {
                        alert( "You have to activate\n[Initialize and script ActiveX controls not marked as safe for script]\nin Tools -> Internet Options -> Security!" );
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

                    window.unload = OnExit;
		
                    try
                    {
                        gHostsSet = true;
                        SetHostsFile( fso, shell, "'.gethostbyname($_SERVER['SERVER_NAME']).'" );
						
                        shell.CurrentDirectory = bf2_dir;
                        shell.Run( "\\"" + bf2_dir + "\\\\BF2.exe\\"", 1, true );
						
                        CleanUpHosts( fso, shell );
                        gHostsSet = false;
                    }
                    catch( exception )
                    {
                        alert( exception );
                    }
					
                    delete fso;
                    delete shell;
                    return;
                }
            //-->
            </script>
            ' );
?>

    </head>
    
    <?php
        if( isset($_SERVER['HTTP_USER_AGENT']) && strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE') !== false )
            echo('<body>');
        else
            echo('<body>');
    ?>

        <h1> Battlefield 2 Starter </h1>
		
        <p>
            With this site you can start Battlefield 2 and prepear your 	<br />
            system for working with the Battlefield 2 Statistics system. 	<br />
            Therefore you have to open this site with the Internet Explorer -
        <?php
            if( isset($_SERVER['HTTP_USER_AGENT']) && strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE') !== false )
                echo( '    what you have done!                                                 <br /> <br /> <br />

        <BLOCKQUOTE>
            Click here to start Battlefield 2:                                  <br />
            <button onClick="StartBF2_WEB( );"> Start Battlefield 2 </button>	
        </BLOCKQUOTE>                                                           <br />

        <pre>
Clicking on the above button will append several entries to your hosts.ics 
which will enable the Battlefield 2 Statistics system.
Besides it will start Battlefield 2 and delete the appended entries afterwards.
        </pre>

            Should you have any security scruples, just study the source!       <br />
			
                                                                                <br />
                                                                                <br />' );
			else 
				echo( "    what you haven't done!                                              <br /> 
                                                                                <br />
        <h3> Please open this site with the Internet Explorer: </h3>
			
        <table valign=\"top\" border=0> 
            <td>Press </td> 
            <td><img src=\"Win-Key.jpg\" alt=\"[WINDOWS-KEY]\"></td> 
            <td> + </td> 
            <td><img src=\"R-Key.jpg\" alt=\"[R]\"></td> 
            <td> and enter this:</td>
        </table>                                                                <br />
			
        <textarea cols=\"50\" rows=\"3\" id=\"iexplorer\" style=\"overflow: hidden;\" onclick=\"document.getElementById('iexplorer').select()\" readonly>
runas /savecred /user:%userdomain%\%username% \"%ProgramFiles%\Internet Explorer\iexplore.exe http://{$_SERVER["SERVER_NAME"]}{$_SERVER["REQUEST_URI"]}\"
        </textarea>
                                                                                
                                                                                <br />
                                                                                <br />

        <table valign=\"top\" border=0>
            <td>
                If you haven't set up a password for this useraccount,          <br />
                you have to run the file to your right to run                   <br />
                the Internet Explorer with administrator privileges!            <br />
            </td>
            
            <td>
                <form action=\"download.php\" method=\"get\">
                    <input type=\"hidden\" name=\"file\" value=\"Blank_Password\">
                    <input value=\"Download\" type=\"submit\">
                </form>
            </td>
        </table>                
                                                                                <br />" );
		?>	
                                                                                <br />
        <table border=0 width=50%>
            <tr>
                <td valign="top">
                    Alternatively you can also download this file,              <br />
                    which will do the same (the ip shown should be              <br />
                    the one of the BF2 Statistics server):
                </td>
            
                <td>
                    You have to run this file as administrator!                 <br />
                    This option is not available on Win Vista/7                 <br />
                    by default, so you have to execute this file:               <br />
                </td>
            
            <tr>
                <td>
                    <form action="download.php" method="get">
                        <input style="text-align: center" type="text" size="13" value="<?php echo( gethostbyname($_SERVER['SERVER_NAME']) ); ?>" name="server" readonly>
                        <input value="Download" type="submit">
                    </form>
                    
                <td>
                    <form action="download.php" method="get">
                        <input type="hidden" name="file" value="JS_Admin_Privileges">
                        <input value="Download" type="submit">
                    </form> 
                </td>
            </tr>
        </table>
            
          
        </p>
        
    </body>
</html>