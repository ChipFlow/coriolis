// -*- C++ -*-
//
// This file is part of the Coriolis Software.
// Copyright (c) Sorbonne Université 2008-2021, All Rights Reserved
//
// +-----------------------------------------------------------------+ 
// |                   C O R I O L I S                               |
// |          Alliance / Hurricane  Interface                        |
// |                                                                 |
// |  Author      :                    Jean-Paul CHAPUT              |
// |  E-mail      :            Jean-Paul.Chaput@lip6.fr              |
// | =============================================================== |
// |  C++ Module  :   "./Utilities.cpp"                              |
// +-----------------------------------------------------------------+


#include  <Python.h>
#include  <csignal>
#include  <cstdlib>
#include  <cstring>
#include  <iomanip>

#include  <boost/program_options.hpp>
namespace boptions = boost::program_options;

#include  "hurricane/utilities/Path.h"
#include  "hurricane/configuration/Configuration.h"
#include  "hurricane/Backtrace.h"
#include  "hurricane/Warning.h"
#include  "hurricane/viewer/Script.h"
#include  "crlcore/Utilities.h"
#include  "crlcore/AllianceFramework.h"


namespace {

  using namespace Hurricane;
  using namespace CRL;


  void  verboseLevel1Changed ( Cfg::Parameter* p )
  {
    if (p->asBool()) mstream::enable ( mstream::Verbose0|mstream::Verbose1 );
    else             mstream::disable( mstream::Verbose1|mstream::Verbose2 );

  //cerr << "Verbose Level 1: " << boolalpha << p->asBool() << " mask:" << mstream::getActiveMask() << endl;
  }


  void  verboseLevel2Changed ( Cfg::Parameter* p )
  {
    if (p->asBool()) mstream::enable ( mstream::Verbose0|mstream::Verbose1|mstream::Verbose2 );
    else             mstream::disable( mstream::Verbose2 );

  //cerr << "Verbose Level 2: " << boolalpha << p->asBool() << " mask:" << mstream::getActiveMask() << endl;
  }


  void  infoChanged ( Cfg::Parameter* p )
  {
    if (p->asBool()) mstream::enable ( mstream::Info );
    else             mstream::disable( mstream::Info );
  }


  void  paranoidChanged ( Cfg::Parameter* p )
  {
    if (p->asBool()) mstream::enable ( mstream::Paranoid );
    else             mstream::disable( mstream::Paranoid );
  }


  void  bugChanged ( Cfg::Parameter* p )
  {
    if ( p->asBool() ) mstream::enable ( mstream::Bug );
    else               mstream::disable( mstream::Bug );
  }


  void  catchCoreChanged ( Cfg::Parameter* p )
  { System::setCatchCore( p->asBool() ); }


  void  logModeChanged ( Cfg::Parameter* p )
  {
    if (not p->asBool()) tty::enable  ();
    else                 tty::disable ();
  }


  void  minTraceLevelChanged ( Cfg::Parameter* p )
  {
  //cerr << "minTraceLevelChanged:" << p->asInt() << endl;
    cdebug.setMinLevel( p->asInt() );
  }


  void  maxTraceLevelChanged ( Cfg::Parameter* p )
  {
  //cerr << "maxTraceLevelChanged:" << p->asInt() << endl;
    cdebug.setMaxLevel( p->asInt() );
  }


  void  stratus1MappingNameChanged ( Cfg::Parameter* p )
  {
    Utilities::Path stratusMappingName ( p->asString() );
    if ( not stratusMappingName.absolute() ) {
      stratusMappingName = System::getPath("etc") / stratusMappingName;
    }
    setenv ( "STRATUS_MAPPING_NAME", stratusMappingName.toString().c_str(), 1 );
  }


  std::string  environmentMapper ( std::string environmentName )
  {
    if      ( environmentName == "HOME"                 ) return "home";
    else if ( environmentName == "CORIOLIS_TOP"         ) return "coriolis_top";
    else if ( environmentName == "STRATUS_MAPPING_NAME" ) return "stratus_mapping_name";
    return "";
  }


}  // End of anonymous namespace.


int           tty::_width          = 80;
bool          tty::_enabled        = true;
unsigned int  mstream::_activeMask = mstream::PassThrough|mstream::Verbose0;

mstream  cmess0    ( mstream::Verbose0, std::cout );
mstream  cmess1    ( mstream::Verbose1, std::cout );
mstream  cmess2    ( mstream::Verbose2, std::cout );
mstream  cinfo     ( mstream::Info    , std::cout );
mstream  cparanoid ( mstream::Paranoid, std::cout );
mstream  cbug      ( mstream::Bug     , std::cout );


// -------------------------------------------------------------------
// Class  :  "::Dots".


Dots::Dots ( const std::string& left, const std::string& right ) : _left(left), _right(right) { }


Dots  Dots::asPercentage ( const std::string& left, float value )
{
  std::ostringstream right;
  right << std::setprecision(3) << (value*100.0) << "%";
  return Dots(left,right.str());
}


Dots  Dots::asBool ( const std::string& left, bool value )
{ std::ostringstream right; right << std::boolalpha << value; return Dots(left,right.str()); }


Dots  Dots::asUInt ( const std::string& left, unsigned int value )
{ std::ostringstream right; right << value; return Dots(left,right.str()); }


Dots  Dots::asInt ( const std::string& left, int value )
{ std::ostringstream right; right << value; return Dots(left,right.str()); }


Dots  Dots::asULong ( const std::string& left, unsigned long value )
{ std::ostringstream right; right << value; return Dots(left,right.str()); }


Dots  Dots::asSizet ( const std::string& left, size_t value )
{ std::ostringstream right; right << value; return Dots(left,right.str()); }


Dots  Dots::asDouble ( const std::string& left, double value )
{ std::ostringstream right; right << value; return Dots(left,right.str()); }


Dots  Dots::asLambda ( const std::string& left, Hurricane::DbU::Unit value )
{ std::ostringstream right; right << Hurricane::DbU::getValueString(value); return Dots(left,right.str()); }


Dots  Dots::asLambda ( const std::string& left, double value )
{ std::ostringstream right; right << Hurricane::DbU::getValueString(value); return Dots(left,right.str()); }


Dots  Dots::asIdentifier ( const std::string& left, const std::string& value )
{ std::ostringstream right; right << "<" << value << ">"; return Dots(left,right.str()); }


Dots  Dots::asString ( const std::string& left, const std::string& value )
{ return Dots(left,value); }


std::ostream& operator<< ( std::ostream& out, const Dots& dots )
{
  int count = tty::getWidth() - 2 - dots._left.length() - dots._right.length();
  out << dots._left << " "; while ( count-- > 0 ) out << "."; out << " " << dots._right;

  return out;
}



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

// -------------------------------------------------------------------
// Class  :  "CRL::System".


  System* System::_singleton = NULL; //System::get();


  System::System ()
    : _catchCore(true)
  {
  // Immediate setup to avoid some tiresome looping...
    _singleton = this;

  // Set the trap function for the SIGINT signal (CTRL-C).
  //if ( signal(SIGINT,System::TrapSig) == SIG_ERR )
  //  System::TrapSig ( SIGTFLT );

  // Set the trap function for SIGFPE, SIGBUS, SIGABRT and SIGSEGV signals.
    if (   ( signal(SIGFPE , System::_trapSig) == SIG_ERR )
        || ( signal(SIGBUS , System::_trapSig) == SIG_ERR )
        || ( signal(SIGABRT, System::_trapSig) == SIG_ERR )
        || ( signal(SIGPIPE, System::_trapSig) == SIG_ERR )
        || ( signal(SIGSEGV, System::_trapSig) == SIG_ERR ) )
      System::_trapSig ( SIGTFLT );

  // Environment variables reading.
    boptions::options_description options ("Environment Variables");
    options.add_options()
      ( "home"                , boptions::value<string>()
                              , "User's home directory." )
      ( "coriolis_top"        , boptions::value<string>()->default_value(CORIOLIS_TOP)
                              , "The root directory of the Coriolis installation tree." )
      ( "stratus_mapping_name", boptions::value<string>()
                              , "Stratus virtual cells mapping." );

    boptions::variables_map arguments;
    boptions::store  ( boptions::parse_environment(options,environmentMapper), arguments );
    boptions::notify ( arguments );

  // Force creation of singleton at this stage.
  // cerr << "In System singleton creation." << endl;
  // AllianceFramework::get();
  // cerr << "AllianceFramework has been allocated." << endl;

  // cerr << "std::string typeid name:" << typeid(string).name() << endl;

  // Check for duplicated type_info initialization.
    const boptions::variable_value& value = arguments["coriolis_top"];
    if ( value.value().type() != typeid(string) ) {
      throw Error("type_info RTTI tree has been initialized twice.\n\n"
                  "        This may be due to incorrect import of Python modules, please ensure\n"
                  "        that the CRL module is always imported first."
                 );
    }

    if ( arguments.count("coriolis_top") ) {
      _pathes.insert ( make_pair("coriolis_top", arguments["coriolis_top"].as<string>()) );
    }

    Utilities::Path sysConfDir ( SYS_CONF_DIR );
    if ( not sysConfDir.absolute() ) {
      if ( arguments.count("coriolis_top") ) {
        // const boptions::variable_value& value = arguments["coriolis_top"];
        // cerr << "value:"
        //      << " empty:"     << boolalpha << value.empty()
        //      << " defaulted:" << boolalpha << value.defaulted()
        //      << endl;
        // const type_info& info = value.value().type();
        // cerr << "type_info:" << info.name()
        //      << " vs. " << typeid(string).name() << endl;
        // cerr << "Equal:" << boolalpha << (info == typeid(std::string)) << endl;

        // const type_info& info2 = typeid(string);
        // cerr << (void*)&(typeid(string))
        //      << " vs. " << (void*)&info2
        //      << " vs. " << (void*)&info
        //      << endl;
        // cerr << "any_cast<string>:" << boost::any_cast<string>(value.value()) << endl;

        sysConfDir = arguments["coriolis_top"].as<string>() / sysConfDir;
      } else {
        cerr << Error("Environment variable CORIOLIS_TOP not set,"
                      " may be unable to read configuration...") << endl;
      }
    }
    sysConfDir /= "coriolis2";
    _pathes.insert ( make_pair("etc" ,sysConfDir                    ) );
    _pathes.insert ( make_pair("home",arguments["home"].as<string>()) );

  // Early setting of python pathes to be able to execute configuration scripts.
    Utilities::Path pythonSitePackages ( PYTHON_SITE_PACKAGES );
    pythonSitePackages = arguments["coriolis_top"].as<string>() / pythonSitePackages;
    _pathes.insert ( make_pair("pythonSitePackages",pythonSitePackages.toString()) );
    // Utilities::Path crlcoreDir  = pythonSitePackages / "crlcore";
    // Utilities::Path stratusDir  = pythonSitePackages / "stratus";
    // Utilities::Path cumulusDir  = pythonSitePackages / "cumulus";
    // Utilities::Path oroshiDir   = pythonSitePackages / "oroshi";
    // Utilities::Path karakazeDir = pythonSitePackages / "karakaze";
    Utilities::Path etcDir      = _pathes["etc"];

    Isobar::Script::addPath ( etcDir.toString() );
    Isobar::Script::addPath ( sysConfDir.toString() );
    Isobar::Script::addPath ( pythonSitePackages.toString() );
    // Isobar::Script::addPath ( crlcoreDir.toString() );
    // Isobar::Script::addPath ( stratusDir.toString() );
    // Isobar::Script::addPath ( cumulusDir.toString() );
    // Isobar::Script::addPath ( oroshiDir.toString() );
    // Isobar::Script::addPath ( karakazeDir.toString() );

  // Triggers Configuration singleton creation.
    Cfg::Configuration::get ();

    Cfg::getParamBool  ("misc.catchCore"      ,false )->registerCb ( this, catchCoreChanged );
    Cfg::getParamBool  ("misc.verboseLevel1"  ,false )->registerCb ( this, verboseLevel1Changed );
    Cfg::getParamBool  ("misc.verboseLevel2"  ,false )->registerCb ( this, verboseLevel2Changed );
    Cfg::getParamBool  ("misc.info"           ,false )->registerCb ( this, infoChanged );
    Cfg::getParamBool  ("misc.paranoid"       ,false )->registerCb ( this, paranoidChanged );
    Cfg::getParamBool  ("misc.bug"            ,false )->registerCb ( this, bugChanged );
    Cfg::getParamBool  ("misc.logMode"        ,false )->registerCb ( this, logModeChanged );
    Cfg::getParamInt   ("misc.minTraceLevel"  ,100000)->registerCb ( this, minTraceLevelChanged );
    Cfg::getParamInt   ("misc.maxTraceLevel"  ,0     )->registerCb ( this, maxTraceLevelChanged );
    Cfg::getParamString("stratus1.mappingName",""    )->registerCb ( this, stratus1MappingNameChanged );

  // Immediate update from the configuration.
  //catchCoreChanged     ( Cfg::getParamBool("misc.catchCore"    ) );
  //verboseLevel1Changed ( Cfg::getParamBool("misc.verboseLevel1") );
  //verboseLevel2Changed ( Cfg::getParamBool("misc.verboseLevel2") );
  //infoChanged          ( Cfg::getParamBool("misc.info"         ) );
  //paranoidChanged      ( Cfg::getParamBool("misc.paranoid"     ) );
  //bugChanged           ( Cfg::getParamBool("misc.bug"          ) );
  //logModeChanged       ( Cfg::getParamBool("misc.logMode"      ) );
  //traceLevelChanged    ( Cfg::getParamInt ("misc.traceLevel"   ) );

    Utilities::Path stratusMappingName;
    if ( arguments.count("stratus_mapping_name") ) {
      Cfg::getParamString( "stratus1.mappingName")->setString ( arguments["stratus_mapping_name"].as<string>() );
    }
  }


  System *System::get ()
  {
    if ( _singleton == NULL ) {
      _singleton = new System ();
    }

    return _singleton;
  }


  void  System::_trapSig ( int sig )
  {
    cerr << "\n\n[CRL ERROR] System::_trapSig(): sig=" << sig << "\n" << endl;
    cerr << "Program stack:\n" << Backtrace(true).textWhere() << endl;

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

        if (sig == SIGABRT) cerr << "(SIGABRT).";
        if (sig == SIGALRM) cerr << "(SIGALARM).";
        if (sig == SIGBUS ) cerr << "(SIGBUS).";
        if (sig == SIGCHLD) cerr << "(SIGCHLD).";
        if (sig == SIGCONT) cerr << "(SIGCONT).";
        if (sig == SIGFPE ) cerr << "(SIGFPE).";
        if (sig == SIGHUP ) cerr << "(SIGHUP).";
        if (sig == SIGILL ) cerr << "(SIGILL).";
        if (sig == SIGINT ) cerr << "(SIGINT).";
        if (sig == SIGIO  ) cerr << "(SIGIO).";
        if (sig == SIGKILL) cerr << "(SIGKILL).";
        if (sig == SIGPIPE) cerr << "(SIGPIPE).";
        if (sig == SIGQUIT) cerr << "(SIGQUIT).";
        if (sig == SIGSEGV) cerr << "(SIGSEGV).";
        if (sig == SIGSTOP) cerr << "(SIGSTOP).";
        if (sig == SIGTSTP) cerr << "(SIGTSTP).";
        if (sig == SIGSYS ) cerr << "(SIGSYS).";
        if (sig == SIGTERM) cerr << "(SIGTERM).";
        if (sig == SIGTRAP) cerr << "(SIGTRAP).";

        cerr << "\n  Please e-mail to <alliance-users@asim.lip6.fr>.\n"
             << "\n  program terminated ";

        if ( getCatchCore() ) {
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


  const Utilities::Path& System::_getPath ( const string& key )
  {
    static Utilities::Path nullPath ("no_path");

    map<const string,Utilities::Path>::const_iterator ipath = _pathes.find( key );
    if ( ipath == _pathes.end() ) return nullPath;

    return (*ipath).second;
  }


  void  System::_runPythonInit ()
  {
    Utilities::Path sysConfDir         = getPath("etc");
    Utilities::Path pythonSitePackages = getPath("pythonSitePackages");

  //bool            systemConfFound = false;
    Utilities::Path systemConfFile  = pythonSitePackages / "crlcore" / "coriolisInit.py";
    if ( systemConfFile.exists() ) {
    //systemConfFound = true;
    //cout << "  o  Reading python dot configuration:" << endl;
    //cout << "     - <" << systemConfFile.string() << ">." << endl;

      Isobar::Script* systemScript = Isobar::Script::create(systemConfFile.stem().toString());
      systemScript->runFunction("coriolisConfigure",NULL,Isobar::Script::NoScriptArgs);
      systemScript->destroy();
    } else {
      cerr << Warning("System configuration file:\n  <%s> not found."
                     ,systemConfFile.toString().c_str()) << endl;
    }

  // Delayed printing, as we known only now whether VerboseLevel1 is requested.
  //if ( cmess1.enabled() ) {
  //  cmess1 << "  o  Reading Configuration. " << endl;
  //  if (systemConfFound) cmess1 << "     - <" << systemConfFile.string() << ">." << endl;
  //  if (homeConfFound)   cmess1 << "     - <" << homeConfFile.string() << ">." << endl;
  //  if (dotConfFound)    cmess1 << "     - <" << dotConfFile.string() << ">." << endl;
  //}
  }


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
