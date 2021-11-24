#include "node.h"
#include <iostream>

using namespace std;

bool settled = false;

void printRT(vector<RoutingNode*> nd){
/*Print routing table entries*/
  for (int i = 0; i < nd.size(); i++) {
    nd[i]->printTable();
  }
}

void routingAlgo(vector<RoutingNode*> nd){
  while(!settled){
    // initialized assuming tables have not converged yet
    // settled will be updated in recvMsg
    settled = true;

    // send message for all the nodes
    for (int i =0 ; i < nd.size(); i++) {
      nd[i]->sendMsg();
    }
  }

  /*Print routing table entries after routing algo converges */
  printRT(nd);
}


void RoutingNode::recvMsg(RouteMsg *msg) {
  int n = (msg -> mytbl)->tbl.size();

  for (int i = 0; i < n; ++i) {
    // flag for indicating if we have a entry for this ip in the table already
    bool found = false;

    // iterate through the table, and look for existing extry with the same destination ip
    for (int j = 0; j < mytbl.tbl.size(); ++j) {
      if((msg -> mytbl) -> tbl[i].dstip != mytbl.tbl[j].dstip)
        continue;

      // entry with matching destinatino IP found
      found = true;

      if(min(1 + (msg -> mytbl)->tbl[i].cost, 16) < mytbl.tbl[j].cost){
        // we need to update entry as new cost is less
        settled = false;

        // dest same as original entry
        mytbl.tbl[j].nexthop = msg->from;
        mytbl.tbl[j].ip_interface = msg->recvip;
        mytbl.tbl[j].cost = min(1 + (msg -> mytbl) -> tbl[i].cost, 16);
      }
      else if(mytbl.tbl[j].nexthop == msg -> from){
        // if next hop has updated cost, update
        if(min(1 + (msg -> mytbl)->tbl[i].cost, 16) != mytbl.tbl[j].cost) {
          settled = false;
          mytbl.tbl[j].cost = min(1 + (msg -> mytbl)->tbl[i].cost, 16);
        }
      }

      break;
    }

    if(!found) {
      // create a new entry
      RoutingEntry new_entry;

      new_entry.dstip = (msg -> mytbl) -> tbl[i].dstip;
      new_entry.ip_interface = msg->recvip;
      new_entry.nexthop = msg->from;
      new_entry.cost = min((msg -> mytbl)->tbl[i].cost + 1, 16);

      mytbl.tbl.push_back(new_entry);
      settled=false;
    }
  }
}




