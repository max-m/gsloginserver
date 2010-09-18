GS_Login_Emulator.exe AnyConfig.cfg

-> Starts the Emulator with the values of this CFG
-> By default "gs_server.cfg" will be loaded (so you simply can start the emulator)


dont forget to edit your hosts.ics located in %systemroot%\system32\drivers\etc or set up an DNS-Server that 
redirects this entries to a server which runs the emulator. 

The neccessary entries in host.ics are:

127.0.0.1 gpcm.gamespy.com
127.0.0.1 gpsp.gamespy.com
127.0.0.1 battlefield2.available.com

Support and help will soon be available on:
http://www.bf2statistics.com


The default database is SQLite (file-based), if you want to use MySQL edit the config (default: gs_server.cfg).
If you dont know what database you should take, do not edit the config - the emulator work in the condition you
got it!
MySQL users also have to execute mysql_create_tables.sql to create the tables the emulator is using.


Special thanks go to:
 - Aluigi for the idea
 - Sajiki for server sponsoring and tests
 - Trevor Hogan for the Server idea and his masterpiece GHost++