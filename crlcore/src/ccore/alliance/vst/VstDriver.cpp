// This file is part of the Coriolis Project.
// Copyright (C) Laboratoire LIP6 - Departement ASIM
// Universite Pierre et Marie Curie
//
// Main contributors :
//        Christophe Alexandre   <Christophe.Alexandre@lip6.fr>
//        Sophie Belloeil             <Sophie.Belloeil@lip6.fr>
//        Hugo Cl�ment                   <Hugo.Clement@lip6.fr>
//        Jean-Paul Chaput           <Jean-Paul.Chaput@lip6.fr>
//        Damien Dupuis                 <Damien.Dupuis@lip6.fr>
//        Christian Masson           <Christian.Masson@lip6.fr>
//        Marek Sroka                     <Marek.Sroka@lip6.fr>
// 
// The  Coriolis Project  is  free software;  you  can redistribute it
// and/or modify it under the  terms of the GNU General Public License
// as published by  the Free Software Foundation; either  version 2 of
// the License, or (at your option) any later version.
// 
// The  Coriolis Project is  distributed in  the hope that it  will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY  or FITNESS FOR  A PARTICULAR PURPOSE.   See the
// GNU General Public License for more details.
// 
// You should have  received a copy of the  GNU General Public License
// along with the Coriolis Project; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
//
// License-Tag
//
// Date   : 01/10/2004
// Author : Christophe Alexandre  <Christophe.Alexandre@lip6.fr>
//
// Authors-Tag

#include  "hurricane/Cell.h"
#include  "hurricane/Net.h"
#include  "hurricane/Instance.h"
#include  "hurricane/Plug.h"

using namespace Hurricane;

#include  "crlcore/Catalog.h"
#include  "crlcore/NetExtension.h"
#include  "Vst.h"

namespace {


  using namespace CRL;


void FilterPointsInStrings(string& s) {
    // Problem in VST... : the . which seems to represent hierachy paths
    // in ap is not supported...
    // So for the moment we just replace . by _ ... Hope this works
    // If you are reading this message then maybe it does not!!
    string::size_type iterator = s.find('.');
    while (iterator != string::npos)
    {
        s[iterator] = '_';
        iterator = s.find('.', iterator);
    }
}

struct StringSort
{

    // Sorts signals taking into account parenthesis ...
    // i(0) is before i(10)
    bool operator()(const string* string1, const string* string2) const
    {
        string::size_type string1OpenPar = string1->find('(');
        string::size_type string2OpenPar = string2->find('(');
        if ((string1OpenPar == string::npos) || (string2OpenPar == string::npos))
            return (*string1 < *string2);
        if ((string1->find('(', string1OpenPar + 1) != string::npos)
                || (string2->find('(', string2OpenPar + 1) != string::npos))
            throw Error("malformed string, multi '('");
        string::size_type string1ClosePar = string1->rfind(')');
        string::size_type string2ClosePar = string2->rfind(')');
        if ((string1ClosePar == string::npos) || (string2ClosePar == string::npos))
            throw Error("malformed string, cannot find ')'");
        if ((string1ClosePar != string1->size()-1) || (string2ClosePar != string2->size()-1)) {
            cerr << "string1 := \"" << *string1 << "\"" << endl;
            cerr << "string2 := \"" << *string2 << "\"" << endl;
            throw Error("malformed string");
        }
        if ((string1->rfind(')', string1ClosePar - 1) != string::npos)
                || (string2->rfind(')', string2ClosePar - 1) != string::npos))
            throw Error("malformed string, multi ')'");

        if (string1->compare(0, string1OpenPar + 1, *string2, 0, string2OpenPar + 1))
            return (*string1 < *string2);

        string number1String = string(*string1, string1OpenPar + 1, string1->size()-string1OpenPar-2);
        string number2String = string(*string2, string2OpenPar + 1, string2->size()-string2OpenPar-2);

        int number1 = atoi(number1String.c_str());
        int number2 = atoi(number2String.c_str());

        return (number1 < number2);
    }
};

unsigned FindIndex(const string& stringtosearch, string::size_type openpar)
{
    string numberString = string(stringtosearch, openpar+1, stringtosearch.size()-openpar-2);
    return atoi(numberString.c_str());
}

string getNetDirection(const Net* net) {
    switch (net->getDirection()) {
        case Net::Direction::UNDEFINED:
            return ("linkage");
        case Net::Direction::IN:
            return ("in");
        case Net::Direction::OUT:
        case Net::Direction::TRISTATE:
            return ("out");
        case Net::Direction::INOUT:
            return ("inout");
        default:
            throw Error("Unrecognized direction");
    }
}

typedef vector<string*> StringPtVector;
typedef vector<string> StringVector;
typedef map<const string*, Net*> StringNetMap; 
typedef map<const string*, Plug*> StringPlugMap;
typedef set<Net*> NetSet; 
typedef vector<Plug*> PlugVector;

unsigned FindBiggestName(StringPtVector& stringptvector)
{
    unsigned biggest = 0;
    for (StringPtVector::const_iterator spvit = stringptvector.begin();
            spvit != stringptvector.end();
            spvit++)
    {
        string::size_type stringOpenPar = (*spvit)->find('(');
        if (stringOpenPar != string::npos)
        {
            if (biggest < stringOpenPar)
                biggest = stringOpenPar;
        }
        else
            if (biggest < (*spvit)->size())
                biggest = (*spvit)->size();
    }
    return biggest;
}

void DumpPortList(ofstream &ccell, Cell* cell)
{
    ccell << "   port (" << endl;
    StringPtVector netsString;
    StringNetMap stringNetMap;
    for_each_net(net, cell->getExternalNets())
    {
        const Name& name = NetExtension::getPort(net);
        string* stringName;
        if (!name.isEmpty())
          stringName = new string(getString(name));
        else
          stringName = new string(getString(net->getName()));
        netsString.push_back(stringName);
        StringNetMap::iterator snmit = stringNetMap.find(stringName);
        if (snmit != stringNetMap.end())
        {
            throw Error("two times the same name");
        }
        stringNetMap[stringName] = net;
        end_for;
    }
    sort(netsString.begin(), netsString.end(), StringSort());

    unsigned biggestName = FindBiggestName(netsString);
    
    StringVector vectorizedNetsString;
    StringPtVector::const_iterator spvit = netsString.begin();
    while (spvit!=netsString.end())
    {
        const string* string1 = *spvit;
        string::size_type string1OpenPar = string1->find('(');
        StringNetMap::iterator snmit = stringNetMap.find(string1);
        if (snmit == stringNetMap.end())
        {
            throw Error("Cannot find net named " + *string1);
        }
        Net* net1 = snmit->second;
        string bitType;
        if (net1->getDirection() == Net::Direction::TRISTATE)
            bitType = " mux_bit bus";
        else
            bitType = " bit";
            
        if (string1OpenPar == string::npos)
        {
            vectorizedNetsString.push_back(
                "      "
                + *string1
                + string(biggestName - string1->size() + 1, ' ')
                + ":  "
                + getNetDirection(net1)
                + bitType);
            ++spvit;
            continue;
        }

        unsigned index1 = 0;
        unsigned index2 = 0;
        bool vectorFound = false;

        while (++spvit != netsString.end())
        {
            if (!vectorFound)
            {
                index1 = index2 = FindIndex(*string1, string1OpenPar);
            }
            const string* string2 = *spvit;
            StringNetMap::iterator snmit = stringNetMap.find(string2);
            if (snmit == stringNetMap.end())
            {
                throw Error("Cannot find net named " + *string2);
            }
            Net* net2 = snmit->second;
            string::size_type string2OpenPar = string2->find('(');
            if (string2OpenPar == string::npos)
                break;
            if (string1->compare(0, string1OpenPar + 1, *string2, 0, string2OpenPar + 1))
                break;
            if (net1->getDirection() != net2->getDirection())
                break;
            unsigned newIndex = FindIndex(*string2, string2OpenPar);
            if (!vectorFound)
            {
                index1 = FindIndex(*string1, string1OpenPar);
                if (newIndex != index1+1)
                    break;
                index2 = newIndex;
                vectorFound= true;
            }
            else
            {
                if (newIndex != index2+1)
                    break;
                index2 = newIndex;
            }
        }
        ostringstream index1StringStream;
        index1StringStream << index1;
        ostringstream index2StringStream;
        index2StringStream << index2;
        string name = string(*string1, 0, string1OpenPar);
        string vectorType;
        string busType;
        if (net1->getDirection() == Net::Direction::TRISTATE)
        {
            vectorType = " mux_vector(";
            busType = " bus";
        }
        else
            vectorType = " bit_vector(";
        vectorizedNetsString.push_back(
                "      "
                + name
                + string(biggestName - name.size() + 1, ' ')
                + ":  "
                + getNetDirection(net1)
                + vectorType
                + index1StringStream.str()
                + " to "
                + index2StringStream.str()
                + ")"
                + busType); 
    }

    StringVector::const_iterator svit = vectorizedNetsString.begin();
    while ((svit != vectorizedNetsString.end()) && (svit+1 != vectorizedNetsString.end()))
    {
        ccell << *(svit++) << ";" << endl;
    }
    if (svit != vectorizedNetsString.end())
        ccell << *svit << endl;
    ccell << "   );" << endl;
    for (StringPtVector::iterator spvit = netsString.begin();
            spvit != netsString.end();
            spvit++)
    {
        delete(*spvit);
    }
}

void DumpSignalList(ofstream &ccell, Cell* cell)
{
    StringPtVector netsString;
    NetSet netSet;
    for_each_net(net, cell->getInternalNets())
    {
        string* stringName = new string(net->getName()._getString());
        FilterPointsInStrings(*stringName);
        netsString.push_back(stringName);
        NetSet::iterator nsit = netSet.find(net);
        if (nsit != netSet.end())
        {
            throw Error("two times the same name");
        }
        netSet.insert(net);
        end_for;
    }
    sort(netsString.begin(), netsString.end(), StringSort());

    unsigned biggestName = FindBiggestName(netsString);
    
    StringPtVector::const_iterator spvit = netsString.begin();
    while (spvit!=netsString.end())
    {
        const string* string1 = *spvit;
        string::size_type string1OpenPar = string1->find('(');

        if (string1OpenPar == string::npos)
        {
            ccell << "signal "
                << *string1
                << string(biggestName - string1->size() + 1, ' ')
                << ": bit;"
                << endl;
            ++spvit;
            continue;
        }

        unsigned index1 = FindIndex(*string1, string1OpenPar);
        unsigned index2 = index1;

        while (++spvit != netsString.end())
        {
            const string* string2 = *spvit;
            string::size_type string2OpenPar = string2->find('(');
            if (string2OpenPar == string::npos)
                break;
            if (string1->compare(0, string1OpenPar + 1, *string2, 0, string2OpenPar + 1))
                break;
            unsigned newIndex = FindIndex(*string2, string2OpenPar);
            //if (newIndex != index2+1)
            //    break;
            index2 = newIndex;
        }
        ostringstream index1StringStream;
        index1StringStream << index1;
        ostringstream index2StringStream;
        index2StringStream << index2;
        string name = string(*string1, 0, string1OpenPar);
        ccell << "signal "
            << name
            << string(biggestName - name.size() + 1, ' ')
            << ": bit_vector("
                << index1StringStream.str()
                << " to "
                << index2StringStream.str()
                << ");"
                << endl;
    }

    for (StringPtVector::iterator spvit = netsString.begin();
            spvit != netsString.end();
            spvit++)
    {
        delete(*spvit);
    }
}

void DumpConnectionList(ofstream &ccell, Instance*instance)
{
    ccell << "   port map ( " << endl;

    StringPtVector netsString;
    StringPlugMap stringPlugMap;
    for_each_plug(plug, instance->getPlugs())
    {
        Net* masterNet = plug->getMasterNet();
        const Name& name = NetExtension::getPort(masterNet);
        string* netName;
        if (!name.isEmpty())
          netName = new string(getString(name));
        else
          netName = new string(getString(masterNet->getName()));
        netsString.push_back(netName);
        StringPlugMap::iterator spmit = stringPlugMap.find(netName);
        if (spmit != stringPlugMap.end())
        {
            throw Error("two times the same name");
        }
        stringPlugMap[netName] = plug;
        end_for;
    }

    sort(netsString.begin(), netsString.end(), StringSort());

    unsigned biggestName = FindBiggestName(netsString);

    StringVector connectionsString;
    StringPtVector::const_iterator spvit = netsString.begin();
    while (spvit!=netsString.end())
    {
        const string* string1 = *spvit;
        string::size_type string1OpenPar = string1->find('(');
        StringPlugMap::iterator spmit = stringPlugMap.find(string1);
        if (spmit == stringPlugMap.end())
        {
            throw Error("Cannot find net named " + *string1);
        }
        if (string1OpenPar == string::npos)
        {
            Plug* plug = spmit->second;
            string connectedNetName;
            if (plug->isConnected()) {
                Net* net = plug->getNet();
                if (net->isExternal()) {
                    const Name& name = NetExtension::getPort(net);
                    if (!name.isEmpty())
                      connectedNetName = getString(name);
                    else
                      connectedNetName = getString(net->getName());
                }
                else
                  connectedNetName = getString(net->getName());
            } else {
                Net* masterNet = plug->getMasterNet();
                if (masterNet->isGlobal()) // connection by name
                {
                    for_each_net(globalnet, instance->getCell()->getGlobalNets())
                    {
                        if (globalnet->getName() == masterNet->getName())
                        {
                            connectedNetName = string(masterNet->getName()._getString());
                            break;
                        }
                        end_for;
                    }
                    if (connectedNetName == string())
                    {
                        throw Error("No global net " + masterNet->getName()._getString() + " in cell " + instance->getCell()->getName()._getString()); 
                    }
                 
                }
                else
                {
                    throw Error("Plug " + masterNet->getName()._getString() + " of instance " + instance->getName()._getString() + " must be connected");
                }
            }
            FilterPointsInStrings(connectedNetName);
            
            connectionsString.push_back(
                "      "
                + *string1
                + string(biggestName - string1->size() + 1, ' ')
                + " => "
                + connectedNetName
                );
            ++spvit;
            continue;
        }

        Plug* plug1 = spmit->second;
        unsigned index1 = 0;
        unsigned index2 = 0;
        PlugVector plugConnectedVector;
        plugConnectedVector.push_back(plug1);
        bool vectorFound = false;

        while (++spvit != netsString.end())
        {
            const string* string2 = *spvit;
            StringPlugMap::iterator spmit = stringPlugMap.find(string2);
            if (spmit == stringPlugMap.end())
            {
                throw Error("Cannot find plug named " + *string2);
            }
            string::size_type string2OpenPar = string2->find('(');
            if (string2OpenPar == string::npos)
                break;
            if (string1->compare(0, string1OpenPar + 1, *string2, 0, string2OpenPar + 1))
                break;
            unsigned newIndex = FindIndex(*string2, string2OpenPar);
            if (!vectorFound)
            {
                index1 = FindIndex(*string1, string1OpenPar);
                if (newIndex != index1+1)
                    break;
                index2 = newIndex;
                Plug* plug2 = spmit->second;
                plugConnectedVector.push_back(plug2);
                vectorFound= true;
            }
            else
            {
                if (newIndex != index2+1)
                    break;
                index2 = newIndex;
                Plug* plug2 = spmit->second;
                plugConnectedVector.push_back(plug2);
            }
        }

        if (vectorFound)
        {
            if (index2 < index1)
                reverse(plugConnectedVector.begin(), plugConnectedVector.end());
            string connections;
            PlugVector::const_iterator pvit = plugConnectedVector.begin();
            while (pvit != plugConnectedVector.end())
            {
                Plug* plug = *pvit; 
                string connectedNetName;
                if (plug->isConnected()) {
                    Net* net = plug->getNet();
                    if (net->isExternal()) {
                        const Name& name = NetExtension::getPort(net);
                        if (!name.isEmpty())
                          connectedNetName = getString(name);
                        else
                          connectedNetName = getString(net->getName());
                    }
                    else
                      connectedNetName = getString(net->getName());
                }
                else
                    if (plug->getMasterNet()->isGlobal())
                    {
                        cerr << "Make all your connections" << endl;
                        exit(1);
                    }
                FilterPointsInStrings(connectedNetName);
                if (pvit+1 == plugConnectedVector.end())
                    connections += connectedNetName;
                else
                    connections += connectedNetName + "&";
                ++pvit;
            }
            string name = string(*string1, 0, string1OpenPar);
            connectionsString.push_back(
                    "      "
                    + name
                    + string(biggestName - name.size() + 1, ' ')
                    + " => "
                    + connections
                    );
        }
        else
        {
            string name;
            if (string1OpenPar != string::npos)
            {
                name = string(*string1, 0, string1OpenPar); 
            }
            else
            {
                name = *string1;
            }

            string connectedNetName;
            if (plug1->isConnected())
            {
                Net* net = plug1->getNet();
                if (net->isExternal())
                {
                    const Name& portName = NetExtension::getPort(net);
                    if (!portName.isEmpty())
                        connectedNetName = getString(portName);
                    else
                        connectedNetName = getString(net->getName());
                }
            }
            else
                if (plug1->getMasterNet()->isGlobal()) {
                    cerr << "Make all your connections" << endl;
                    exit(1);
                }

            FilterPointsInStrings(connectedNetName);
            connectionsString.push_back(
                    "      "
                    + name
                    + string(biggestName - name.size() + 1, ' ')
                    + " =>  "
                    + connectedNetName
                    );
        }
    }

    StringVector::const_iterator csit = connectionsString.begin();
    while (csit != connectionsString.end())
    {
        if (csit+1 == connectionsString.end())
            ccell << *csit << endl;
        else
            ccell << *csit << "," << endl;
        ++csit;
    }
    ccell << "   );" << endl;

    for (StringPtVector::iterator spvit = netsString.begin();
            spvit != netsString.end();
            spvit++)
    {
        delete(*spvit);
    }
}
    
}


namespace CRL {

void  vstDriver ( const string &cellPath, Cell *cell, unsigned int &saveState )
{
    ::std::ofstream  ccell ( cellPath.c_str() );
    ccell << "entity " << cell->getName() << " is" << endl;
    DumpPortList(ccell, cell);
    ccell << "end " << cell->getName() << ";" << endl;
    ccell << endl;
    ccell << "architecture structural of " << cell->getName() << " is" << endl;
    typedef set<Cell*> ModelSet;
    ModelSet modelSet;
    typedef list<Instance*> InstanceList;
    InstanceList instanceList;
    for_each_instance(instance, cell->getInstances())
    {
        Cell* model = instance->getMasterCell();
        CatalogProperty *stateProp =
            static_cast<CatalogProperty*>(model->getProperty(CatalogProperty::getPropertyName()));
        
        if (stateProp && (stateProp->getState()->isFeed()))
            continue;
        
        modelSet.insert(model);
        instanceList.push_back(instance);
        end_for;
    }
    
    for (ModelSet::const_iterator msit = modelSet.begin();
            msit != modelSet.end();
            msit++)
    {
        Cell* model = *msit;
        ccell << "component " << model->getName() << endl;
        DumpPortList(ccell, model);
        ccell << "end component;" << endl;
        ccell << endl;
    }

    DumpSignalList(ccell, cell);

    ccell << endl << "begin" << endl << endl;

    for (InstanceList::const_iterator ilit = instanceList.begin();
            ilit != instanceList.end();
            ilit++)
    {
        Instance* instance = *ilit;
        string insName(instance->getName()._getString());
        FilterPointsInStrings(insName);
        ccell << insName << " : " << instance->getMasterCell()->getName() << endl;

        DumpConnectionList(ccell, instance);
        ccell << endl;
    }
    ccell << "end structural;" << endl;
    ccell.close ();
}

}