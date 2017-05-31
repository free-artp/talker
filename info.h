#ifndef _INFO_H
#define _INFO_H

	#ifdef INFO
		
		extern FILE * infoc;

		#define INFO_INIT() do{ if (infoc = fopen( "/dev/console", "w" )) setbuf(infoc,NULL); } while( 0 )
//		#define INFO_INIT() do{ infoc = stdout; } while( 0 )
		#define INFO_CLS() do{ fprintf( infoc, "\033[2J\033[;H" ); } while( 0 )
		#define INFO_PRINT(...) do{ fprintf( infoc, __VA_ARGS__ ); } while( 0 )
		#define INFO_PRINTLC(line, column, fmt, ...) do{ fprintf( infoc, "\033[%-d;%-dH" fmt, line, column, __VA_ARGS__ ); } while( 0 )

	#else

		#define INFO_INIT() do{ } while ( 0 )
		#define INFO_CLS(...) do{ } while ( 0 )
		#define INFO_PRINT(...) do{ } while ( 0 )
		#define INFO_PRINTLC(...) do{ } while ( 0 )

	#endif



#endif

