#include "Seabreeze/Tree.h"

#include <iostream>
#include <iterator>

using namespace std;

Tree::Tree ()
  :nodes()
{}

Tree::~Tree ()
{
  for(Node* n : nodes){
    delete n;
  }
}

int Tree::get_N ()
{
  return nodes.size();
}

Node* Tree::get_node ( Contact* cont )
{
  Node* node = nullptr;
  for ( Node* n : nodes ) {
    if ( n->_contact == cont ) {
      node = n;
    }
  }

  return node;
}

vector<Node*> Tree::get_node_list ()
{
  return nodes;
}

void Tree::new_node ()
{
  nodes.push_back(new Node());
}

void Tree::add_node ( Node* node )
{
  node->label = nodes.size();
  if ( find(nodes.begin(), nodes.end(), node) == nodes.end() )
    nodes.push_back(node);
}

void Tree::After_i ( Node *ni )
{
  if(ni == nullptr)
    return;
	
  ni->ap = 1;
  for ( Node* ne : ni->Ne ) {
    After_i(ne);
  }
}

set<Node*> Tree::Branch_i ( Contact* ct )
{
  set<Node*> ln;
  Node *ni = get_node(ct);
  while ( ni->Np != nullptr ) {
    ln.insert(ni->Np);
    ni = ni->Np;
  }
	
  return ln;
}

double Tree::Delay_Elmore ( RoutingPad* rp )
{
  if ( rp == nullptr ) {
    cerr << "Input RoutingPad is NULL. Please select a RoutingPad !" << endl;
    return -1;
  }

  Contact* ct = nullptr;
  for ( Component* c : rp->getSlaveComponents() ) {
    Contact* cont = dynamic_cast<Contact*>(c);

    if ( cont ) {
      ct = cont;
      break;
    }
  }
  if ( ct == nullptr ) {
    cerr << "No contact found" << endl;
    return -1;
  }
  
  double t = 0;
//---------------------------------------------------------------------------------------
  cerr << "Contact to be calculated : " << ct->getId() << endl;
//---------------------------------------------------------------------------------------

  set<Node*> br = Branch_i(ct);
  Node *ni = get_node(ct);
  After_i(ni);
  ni->ap = 0;
  // Compute Rt of all nodes
  for ( size_t k = 0; k < nodes.size(); k++ ) {
    if ( k == 0 )
      nodes[k]->Rt = nodes[k]->R;
    else{
      if ( nodes[k]->ap == 0 ) {
        if ( br.count(nodes[k]) > 0 ) {
	  nodes[k]->Rt = (nodes[k]->Np)->Rt + nodes[k]->R;
	}
	else{
	  nodes[k]->Rt = (nodes[k]->Np)->Rt;
	}
      }
      else{
	nodes[k]->Rt = ni->Rt;
      }
    }
  }

  //Compute Elmore delay time 
  for ( size_t k = 0; k < nodes.size(); k++ ) {
    t += (nodes[k]->Rt)*(nodes[k]->C);
  }
  return t;
}

void Tree::print ( ostream& out )
{
  out << "Start printing tree..." << endl;
  out << "Tree has " << nodes.size() << " nodes :" << endl;
  out << nodes[0]->_contact->getId() 
      << " : R = " << nodes[0]->R 
      << ", C = " << nodes[0]->C
      << " -> ";
  for(Node* n : nodes[0]->Ne){
    out << n->_contact->getId() << ", "; 
  }
  out << std::endl;

  for ( size_t i = 1; i < nodes.size(); i++ ) {
    out << nodes[i]->Np->_contact->getId() 
         << " -> " << nodes[i]->_contact->getId() 
         << " : R = " << nodes[i]->R 
         << ", C = " << nodes[i]->C;
    if ( !(nodes[i]->Ne).empty() ) {
      out << " -> ";
      for ( Node* n : nodes[i]->Ne ) {
        out << n->_contact->getId() << ", ";
      }
    }
    else
      out << " |";
    out << endl;
  }
}

void Tree::clear ()  {
  nodes.clear();
}
