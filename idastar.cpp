/*
Copyright 2018 Pierre-Edouard Portier
peportier.me
​
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
​
    http://www.apache.org/licenses/LICENSE-2.0
​
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
)
*/

#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <map>
#include <utility>
#include <list>
#include <functional>
#include <cmath>
#include <algorithm>
#include <utility>
#include <iomanip>
#include <limits>
#include <chrono>
#include "state.cpp"

using namespace std;

typedef pair<int,int> Move;

void
addNeighbor( State &currentState, Move &move, 
             vector< pair< Move,int > > &neighbors,
             list<State>& path)
{
  currentState.doMove(move);
  if( find( path.begin(), path.end(), currentState ) == path.end() )
  {
    neighbors.push_back( make_pair( move, currentState.heuristic(true) ) );
  }
  Move newMove = move;
  newMove.first=move.second;
  newMove.second=move.first;
  currentState.doMove( newMove ); // undo move
}

void
search( State& currentState,
        int          ub, // upper bound over which exploration must stop
        int&         nub,
        list<State>& path,
        list<State>& bestPath,
        int&         nbVisitedState )

{
  nbVisitedState++;

  int f; // under-estimation of optimal length
  int g = path.size() - 1; // size of the current path to currentState

  if( currentState.isFinal() )
  {
    bestPath = path;
    return;
  }

  // generate the neighbors
  //int s = side( currentState );
  vector< pair< Move,int > > neighbors;
  neighbors.clear();
  for(int i=0; i<currentState.getNbStacks();i++)
  {
    if(currentState.getTop(i)!=-1)
    {
      int currentBlock=currentState.getTop(i);
      for(int j=0; j<currentState.getNbStacks();j++) {
        if(j!=i) {
          Move move = make_pair(i,j);
          addNeighbor(currentState, move, neighbors, path);
        }
      }
    }
  }
  // sort the neighbors by heuristic value

  sort( neighbors.begin(), neighbors.end(),[](const pair<Move,int> &left, const pair<Move,int> &right) 
      {
        return left.second < right.second;
      } );

  for( const pair<Move,int> &p : neighbors )
  {
    f = g + 1 + p.second;
    if( f > ub )
    {
      if( f < nub )
      {
        nub = f; // update the next upper bound
      }
    }
    else
    {
      currentState.doMove(p.first);
      path.push_back( currentState );
      search( currentState,ub, nub, path, bestPath, nbVisitedState );
      path.pop_back(); 
      Move newMove = p.first;
      int first=newMove.first;
      newMove.first=newMove.second;
      newMove.second=first;
      currentState.doMove( newMove );// undo move
      if( !bestPath.empty() ) return;
    }
  }
}

void
ida( State&        initialState,
     list<State>&  bestPath, // path from source to destination
     int&          nbVisitedState )
{
  int ub;                      // current upper bound
  int nub = initialState.heuristic(true); // next upper bound
  list<State> path;
  path.push_back( initialState ); // the path to the target starts with the source

  while( bestPath.empty() && nub != numeric_limits<int>::max() )
  {
    ub = nub;
    nub = numeric_limits<int>::max();

    cout << "upper bound: " << ub;
    search( initialState, ub, nub, path, bestPath, nbVisitedState );
    cout << " ; nbVisitedState: " << nbVisitedState << endl;
  }
}

int
main()
{
  State b=State(4,3);
  b.setInitial();
  list<State> bestPath;
  int nbVisitedState = 0;
  
  auto start = std::chrono::high_resolution_clock::now();
  ida( b,bestPath, nbVisitedState );
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  cout << "Elapsed time: " << elapsed.count() << " s\n";
  cout << "nb moves: " << bestPath.size()-1 << endl;
  cout << "nb visited states: " << nbVisitedState << endl;

  for( const State& s : bestPath ) {
    s.display();
  }
  return 0;
}
