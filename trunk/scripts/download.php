<?php    
    if( isset($_GET['server']) )
    {
        header( 'Content-type: application/x-javascript' ); 
        header( 'Content-Disposition: attachment; filename="BF2_Starter.js"' );
    
        $data = ""; 
        $file = fopen( "BF2_Starter.js", "r"); 
        
        while( !feof($file) )
            $data = $data . fgets( $file, 4096 );
        
        fclose( $file );
        
        echo( str_replace('SetHostsFile( fso, shell );', 'SetHostsFile( fso, shell, "'.$_GET['server'].'" );', $data) );    
    }
    else if( isset($_GET['file']) )
    {         
        $file_name = $_GET['file'];
		
		if( $file_name == "JS_Admin_Privileges" || $file_name == "Blank_Password" )
        {
			header( 'Content-type: application/octet-stream' );
            header( 'Content-Disposition: attachment; filename="'.$file_name.'.reg"' );
        
            $data = "";
            $file = fopen( $file_name.'.reg', 'r' );
        
            while( !feof($file) )
                $data = $data . fgets( $file, 4096 );
               
            fclose( $file );
            
            echo( $data );
        }     
    }
?>