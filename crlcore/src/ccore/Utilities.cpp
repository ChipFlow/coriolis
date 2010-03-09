
// -*- C++ -*-
//
// This file is part of the Coriolis Software.
// Copyright (c) UPMC/LIP6 2008-2009, All Rights Reserved
//
// ===================================================================
//
// $Id$
//
// x-----------------------------------------------------------------x 
// |                                                                 |
// |                   C O R I O L I S                               |
// |          Alliance / Hurricane  Interface                        |
// |                                                                 |
// |  Author      :                    Jean-Paul CHAPUT              |
// |  E-mail      :       Jean-Paul.Chaput@asim.lip6.fr              |
// | =============================================================== |
// |  C++ Module  :       "./Utilities.cpp"                          |
// | *************************************************************** |
// |  U p d a t e s                                                  |
// |                                                                 |
// x-----------------------------------------------------------------x



#include  <csignal>
#include  <cstdlib>
#include  <cstring>

#include  "crlcore/Utilities.h"


bool          tty::_enabled        = true;
unsigned int  mstream::_activeMask = mstream::Verbose0;

mstream  cmess0 ( mstream::Verbose0, std::cout );
mstream  cmess1 ( mstream::Verbose1, std::cout );
mstream  cmess2 ( mstream::Verbose2, std::cout );
mstream  cinfo  ( mstream::Info    , std::cout );



namespace CRL {


  using std::cout;
  using std::cerr;
  using std::endl;
  using std::string;
  using std::ostringstream;


# define        SIGTFLT        1


  // Error messages.
  const char* DupSystem          = "\n    Attempt to re-create Alliance System.";
  const char* BadAllocProperty   = "%s::create():\n    Property allocation failed.\n";
  const char* BadCreate          = "%s::create():\n    Memory allocation failed.\n";
  const char* NullDataBase       = "%s:\n\n    The Hurricane DataBase have not been created yet.\n";
  const char* NullTechnology     = "%s:\n\n    The Hurricane DataBase do not contain any technology.\n";
  const char* NullLibrary        = "%s:\n\n    NULL Library given as argument.\n";
  const char* NullCell           = "%s:\n\n    NULL Cell given as argument.\n";
  const char* BadFopen           = "%s:\n\n    Unable to open %s file : \"%s\".\n";
  const char* BadColorValue      = "%s() :\n\n"
                                        "    Invalid color value for color \"%s\" : \"%s\".\n";

  System*     System::_singleton = System::create ();


// -------------------------------------------------------------------
// Class  :  "CRL::System".


  System *System::create ()
  {
    if ( _singleton != NULL ) {
      cerr << "[WARNING] Attempt to re-create System() singleton." << endl;
      return _singleton;
    }

    _singleton = new System ();

  // Set the trap function for the SIGINT signal (CTRL-C).
  //if ( signal(SIGINT,System::TrapSig) == SIG_ERR )
  //  System::TrapSig ( SIGTFLT );

  // Set the trap function for SIGFPE, SIGBUS, SIGABRT and SIGSEGV signals.
    if (   ( signal(SIGFPE , System::trapSig) == SIG_ERR )
        || ( signal(SIGBUS , System::trapSig) == SIG_ERR )
        || ( signal(SIGABRT, System::trapSig) == SIG_ERR )
        || ( signal(SIGPIPE, System::trapSig) == SIG_ERR )
        || ( signal(SIGSEGV, System::trapSig) == SIG_ERR ) )
      System::trapSig ( SIGTFLT );

    return _singleton;
  }


  void  System::trapSig ( int sig )
  {
    cerr << "\n\n[CRL ERROR] System::trapSig():\n" << endl;

    switch ( sig ) {
      case SIGINT:
      // User interrupt with CTRL-C.
      //emergency ();
        break;

      case SIGTERM:
      case SIGFPE:
      case SIGBUS:
      case SIGSEGV:
      case SIGABRT:
      case SIGPIPE:
      //emergency ();

      // Ouch!! This may result from a program bug.
        cerr << "  An program internal bug have occur ";

        if (sig == SIGFPE ) cerr << "(SIGFPE).";
        if (sig == SIGBUS ) cerr << "(SIGBUS).";
        if (sig == SIGSEGV) cerr << "(SIGSEGV).";
        if (sig == SIGPIPE) cerr << "(SIGPIPE).";

        cerr << "\n  Please e-mail to <coriolis-sav@asim.lip6.fr>.\n"
             << "\n  program terminated ";

        if ( getSystem()->getCatchCore() ) {
          cerr << "(core not dumped).\n";
          exit ( 1 );
        } else {
          cerr << "(core will be dumped).\n";
          if (   ( signal(SIGFPE , SIG_DFL) == SIG_ERR )
              || ( signal(SIGBUS , SIG_DFL) == SIG_ERR )
              || ( signal(SIGABRT, SIG_DFL) == SIG_ERR )
              || ( signal(SIGSEGV, SIG_DFL) == SIG_ERR )
              || ( signal(SIGPIPE, SIG_DFL) == SIG_ERR ) )
            exit ( 1 );
          else {
            kill ( getpid(), /*sig*/ SIGSEGV );
            return;
          }
        }
        break;

      default:
      /* Unexpected signal. */
        cerr << "\n  Unexpected signal \'" << sig << "\' in trap function.\n";
        break;
    }

    exit ( 1 );
  }


  System *System::getSystem () { return _singleton; }


// -------------------------------------------------------------------
// Class  :  "CRL::IoFile".


  bool  IoFile::open ( const string& mode )
  {
    if ( isOpen() )
      throw Error ( "IoFile::Open():\n  Attempt to reopen file %s\n", _path.c_str() );

    _mode       = mode;
    _file       = fopen ( _path.c_str(), mode.c_str() );
    _lineNumber = 0;
    _eof        = false;

    return _file;
  }


  void  IoFile::close ()
  {
    if ( isOpen() ) fclose ( _file );
    _file       = NULL;
    _lineNumber = 0;
    _eof        = false;
  }


  char* IoFile::readLine ( char* buffer, size_t length )
  {
    assert ( buffer != NULL );

    if ( eof() ) {
      buffer[0] = '\0';
    } else {
      char* result = fgets ( buffer, length-1, _file );
      if ( !result || feof(_file) ) {
        _eof = true;
        buffer[0] = '\0';
      } else {
        _lineNumber++;
        size_t readLength = strlen ( buffer );
        if ( buffer[readLength-1] == '\n' )
          buffer[readLength-1] = '\0';
      }
    }

    return buffer;
  }


  string  IoFile::_getString () const
  {
    ostringstream s;

    s << "<IoFile " << _path << ">";

    return s.str();
  }


  Record *IoFile::_getRecord () const
  {
    Record* record = new Record ( "<IoFile>" );
    record->add ( getSlot ( "_path", &_path ) );
    return record;
  }


}  // End of CRL namespace.


// -------------------------------------------------------------------
// Class  :  "mstream".


  void  mstream::enable  ( unsigned int mask ) { _activeMask |=  mask; }
  void  mstream::disable ( unsigned int mask ) { _activeMask &= ~mask; }