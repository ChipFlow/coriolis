// This file is part of the Coriolis Software.
// Copyright (c) UPMC 2008-2014, All Rights Reserved
//
// +-----------------------------------------------------------------+ 
// |                   C O R I O L I S                               |
// |          Alliance / Hurricane  Interface                        |
// |      Yacc Grammar for Alliance Structural VHDL                  |
// |                                                                 |
// |  Author      :                    Jean-Paul CHAPUT              |
// |  E-mail      :       Jean-Paul.Chaput@asim.lip6.fr              |
// | =============================================================== |
// |  Yacc        :  "./VstParserGrammar.yy"                         |
// |                                                                 |
// |  This file is based on the Alliance VHDL parser written by      |
// |       L.A. Tabusse, Vuong H.N., P. Bazargan-Sabet & D. Hommais  |
// +-----------------------------------------------------------------+


#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std;

#include  "hurricane/Warning.h"
#include  "hurricane/Net.h"
#include  "hurricane/Cell.h"
#include  "hurricane/Plug.h"
#include  "hurricane/Instance.h"
#include  "hurricane/UpdateSession.h"
using namespace Hurricane;

#include "crlcore/Utilities.h"
#include "crlcore/Catalog.h"
#include "crlcore/AllianceFramework.h"
#include "crlcore/NetExtension.h"
#include "crlcore/Blif.h"
using namespace CRL;


namespace {
  using namespace std;

  void  addSupplyNets ( Cell* cell )
  {
    Net* vss = Net::create ( cell, "vss" );
    vss->setExternal ( true );
    vss->setGlobal   ( true );
    vss->setType     ( Net::Type::GROUND );

    Net* vdd = Net::create ( cell, "vdd" );
    vdd->setExternal ( true );
    vdd->setGlobal   ( true );
    vdd->setType     ( Net::Type::POWER );
  }

  void  connectSupplyNets ( Instance * instance, Cell* design )
  {
    Cell* instcell = instance->getCell();
    if(instcell == NULL){
        throw Error("Instance's cell is null\n");
    }
    Net* vssint = instcell->getNet( "vss" );
    Net* vddint = instcell->getNet( "vdd" );
    cerr << "Got instance nets" << endl;
    Net* vssext = design->getNet( "vss" );
    Net* vddext = design->getNet( "vdd" );
    cerr << "Got cell nets" << endl;
    auto vssplug = instance->getPlug( vssint );
    auto vddplug = instance->getPlug( vddint );
    cerr << "Got plugs: " << vssplug << " and " << vddplug << endl;
    vssplug->setNet( vssext );
    vddplug->setNet( vddext );
    cerr << "Updated nets" << endl;
  }

  void  SetNetType ( Net* net, AllianceFramework * framework )
  {
    if ( framework->isPOWER(net->getName()) ) {
      net->setType   ( Net::Type::POWER  );
      net->setGlobal ( true );
    } else if ( framework->isGROUND(net->getName()) ) {
      net->setType   ( Net::Type::GROUND );
      net->setGlobal ( true );
    } else if ( framework->isCLOCK(net->getName()) ) {
      net->setType   ( Net::Type::CLOCK );
    } else
      net->setType ( Net::Type::LOGICAL );
  }

  enum ParserState{
    EXT     = 0x00,
    MODEL   = 0x01,
    SUBCKT  = 0x02,
    NAMES   = 0x04,
    INPUTS  = 0x08,
    OUTPUTS = 0x16
  };

  struct subckt{
    string cell;
    vector<pair<string, string> > pins;
  };

  struct model{
    string name;
    unordered_map<string, Net::Direction> pins;
    vector<subckt> subcircuits;
    bool operator<(model const & o) const{ return name < o.name; }
  };

}  // End of anonymous namespace.


namespace CRL {


// 
// Can only parse simple, netlist BLIF files generated by Yosys
// Ignores all ".names" and uses only the .subckt, .model, .input and .output 
//

Cell * Blif::load ( string cellPath ) //, Cell *cell )
{
  using namespace std;

  auto framework = AllianceFramework::get ();

  std::ifstream ccell ( cellPath+".blif" );

  cmess2 << "     " << tab << "+ " << cellPath << " [BLIF]" << endl;

  std::vector<model> models;
  ParserState state = EXT;
  bool hasName;

  string line;
  while(getline(ccell, line)){
    istringstream linestream(line);
    string before_comment;
    getline(linestream, before_comment, '#');
    istringstream tokens(before_comment);
    string token;
    while(tokens>>token){
      assert(not token.empty());
      if(token[0] == '.'){
        if(token == ".model"){
          if(state != EXT)
            throw Error("Nested model are not supported\n");
          state = MODEL;
          hasName = false;
          models.push_back(model());
        }
        else if(token == ".subckt"){
          if(state == EXT)
            throw Error("Subcircuit without an enclosing model are not supported\n");
          if(state == MODEL and not hasName)
            throw Error("Model has no name\n");
          state = SUBCKT;
          hasName = false;
          models.back().subcircuits.push_back(subckt());
        }
        else if(token == ".names"){
          cerr << Warning("BLIF names are ignored\n");
          if(state == EXT)
            throw Error("Names without an enclosing model are not supported\n");
          if(state == MODEL and not hasName)
            throw Error("Model has no name\n");
          state = NAMES;
          hasName = false;
        }
        else if(token == ".latch"){
          throw Error("Latch constructs are not understood by the parser\n");
        }
        else if(token == ".inputs"){
          if(state == EXT)
            throw Error("Inputs have been found without an enclosing model\n");
          state = INPUTS;
        }
        else if(token == ".outputs"){
          if(state == EXT)
            throw Error("Outputs have been found without an enclosing model\n");
          state = OUTPUTS;
        }
        else if(token == ".end"){
          if(state == EXT)
            throw Error("A .end has been found out of a model\n");
          state = EXT;
        }
        else{
          throw Error("Unexpected control token\n");
        }
      }
      else{ // Either a pin or an input/output definition
        if(state == INPUTS or state == OUTPUTS){
          auto it = models.back().pins.find(token);
          Net::Direction D = (state == INPUTS)? Net::Direction::DirIn : Net::Direction::DirOut;
          if(it != models.back().pins.end()){
            it->second = static_cast<Net::Direction::Code>(D | it->second);
          }
          else{
            models.back().pins.insert(pair<string, Net::Direction>(token, D));
          }
        }
        else if(state == SUBCKT){
          if(hasName){
            // Encountered a pin: need to be processed
            istringstream token_stream(token);
            string before_space, after_space;
            getline(token_stream, before_space, '=');
            getline(token_stream, after_space, '=');
            if(token_stream){
                Error("Encountered more than one '=' in token");
            }
            models.back().subcircuits.back().pins.push_back(pair<string, string>(before_space, after_space));
          }
          else{
            models.back().subcircuits.back().cell = token;
            hasName = true;
          }
        }
        else if(state == NAMES){
            // TODO; now just ignored
        }
        else if(state == MODEL){
          if(hasName)
            throw Error("Unexpected token after model name\n");
          else{
            models.back().name = token;
            hasName = true;
          }
        }
        else{
          throw Error("Unexpected token\n");
        }
      }
    }
    line.clear();
  }
  if(state != EXT){
    cerr << Warning("End of model has not been found\n");
  }

  /*
  for(auto & M : models){
    cout << "Model: " << M.name << endl;
    for(auto & S : M.subcircuits){
      cout << "\tInstance of " << S.cell;
      for(auto & P : S.pins){
          cout << " " << P.first << ":" << P.second;
      }
      cout << endl;
    }
  }
  */

  if(models.size() > 1){
    cerr << Warning("Several models in the file; only the last was open\n");
  }

  Cell * design = NULL;
  for(auto M : models){
    design = framework->createCell(M.name);
    addSupplyNets(design);

    unordered_set<string> net_names;
    for(auto const & S : M.subcircuits){
      for(auto const & P : S.pins){
          net_names.insert(P.second);
      }
    }
    for(auto const & P : M.pins){
        net_names.insert(P.first);
    }

    for(string const & N : net_names){
        Net* new_net = Net::create( design, N );
        auto it = M.pins.find(N);
        if(it != M.pins.end()){
            new_net->setExternal( true );
            new_net->setDirection( it->second );
        }
    }

    int i=0;
    for(auto & S : M.subcircuits){
      ostringstream subckt_name;
      subckt_name << "subckt_" << i;
      Cell * cell = framework->getCell(S.cell, Catalog::State::Views, 0);
      if(cell == NULL){
        cerr << Warning("Cell " + S.cell + "to instanciate hasn't been found\n");
        continue;
      }
      Instance* instance = Instance::create( design, subckt_name.str(), cell);

      for(auto & P : S.pins){
        Net* internalNet = cell->getNet( P.first );
        Net* externalNet = design->getNet( P.second );
        assert(internalNet != NULL and externalNet != NULL);
        instance->getPlug( internalNet )->setNet( externalNet );
      }
      //connectSupplyNets(instance, design);
      ++i;
    }
  }

  return design;
}

}


