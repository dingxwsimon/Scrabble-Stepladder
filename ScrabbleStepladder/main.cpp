//
//  main.cpp
//  ScrabbleStepladder
//
//  Created by Sam Bodanis on 28/01/2013.
//  Copyright (c) 2013 Sam Bodanis. All rights reserved.
//

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string.h>
#include <fstream>
#include <ctype.h>
#include <ctime>
#include <queue>
#include <deque>
#include <algorithm>

const bool isDebugging =  true;

using namespace std;

// Stores each word and its corresponding score.
// Score recomputation is avoided this way.
struct vertex {
    string word;
    int score;
};

// This is a struct simply for succinctness in the code.
// Basically a mapping from a vertex, to all adjacient vertices.
struct graph {
    map<vertex *, vector<vertex *> > verticesAndLinks;
};

// Ladder is stored as a deque so that elements can be appended form either side
// with constant time. Also score is stored to avoid repeated recomputation. 
struct ladder {
    deque<vertex *> vertices;
    int score;
};

int scrabbleScore(deque<vertex *> & ladderToCompute);
int computeWordScore(string & wordToCheck);
void printLadder(deque<vertex *> ladder);
vector<string> getInput();
graph makeGraph(vector<string> & sortedWords);
bool isAdjacientVertex(string str1, string str2);
void findLongestLadder(graph & wordGraph);
void searchFromVertex(graph & wordGraph, ladder currentLadder,
                      ladder & longestLadder, set<vertex *> visitedVertices,
                      pair<vertex *, vertex *> newVertices);

// Overloads equality for vertex struct so that nodes are compared by their word.
bool operator==(vertex & lhs, vertex & rhs) {
    return lhs.word == rhs.word;
}

// Less than overload so that vertices can be stored in a map. 
bool operator <(vertex & lhs, vertex & rhs) {
    return lhs.word < rhs.word;
}

int main(int argc, const char * argv[])
{
    vector<string> sortedWords;
    graph wordGraph;
    
    if (isDebugging) {
        int lengthOfWordsToConsider;
        cout << "Enter length of words to consider: ";
        cin >> lengthOfWordsToConsider;
        
        string line;
        ifstream myfile ("allthewords.txt");
        if (myfile.is_open()) {
            while ( myfile.good() ) {
                getline (myfile,line);
                if (line.length() == lengthOfWordsToConsider) {
                    for (int i = 0; i < line.length(); i++) {
                        line[i] = toupper(line[i]);
                    }
                    sortedWords.push_back(line);
                }
            }
            myfile.close();
        } else {
            cout << "Unable to open file";
        }
    
//        sortedWords.push_back("SOUR");
//        sortedWords.push_back("SPUR");
//        sortedWords.push_back("SPUD");
//        sortedWords.push_back("STUD");
//        sortedWords.push_back("STUN");

        clock_t begin = clock();
        wordGraph = makeGraph(sortedWords);
        clock_t end = clock();
        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
        cout << "Time taken: " << elapsed_secs << endl;

    } else {
        sortedWords = getInput();
        wordGraph = makeGraph(sortedWords);
    }
    
    findLongestLadder(wordGraph);
    
    return 0;
}

// For each vertex in the graph, use it as a peak of a ladder
// to search bidirectionally from. 
void findLongestLadder(graph & wordGraph) {
    ladder longestLadder;
    longestLadder.score = 0;
    clock_t begin;
    if (isDebugging) begin = clock();
    
    for (map<vertex *, vector<vertex *> >::iterator currentVertex = wordGraph.verticesAndLinks.begin();
         currentVertex != wordGraph.verticesAndLinks.end(); ++currentVertex) {
        set<vertex *> visitedVertices;
        ladder currentLadder;
        currentLadder.score = 0;
        searchFromVertex(wordGraph, currentLadder, longestLadder, visitedVertices, make_pair(currentVertex->first, currentVertex->first));
    }

    if (isDebugging) {
        clock_t end = clock();
        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
        cout << "\nTime taken: " << elapsed_secs << endl;
        cout << "\nLongest Ladder (" << scrabbleScore(longestLadder.vertices) << ")" << endl;
        printLadder(longestLadder.vertices);
    }
    
    cout << longestLadder.score << endl;
}

// Recursively extend the word ladder from both sides
// Make a prediction of the largest possible ladder score that could
// be made from extending the current ladder. This is done by
// assuming the ladder grows linearly - each step is 1 apart and the min word score is
// every letter with a score of 1. Basically for each side: 
// ladderScore + sum(from: currWordScore, to: minPossibleScore) 
void searchFromVertex(graph & wordGraph, ladder currentLadder,
                      ladder & longestLadder, set<vertex *> visitedVertices, pair<vertex *, vertex *> newVertices) {

    int possibleMax = currentLadder.score;
    int top = newVertices.first->score;
    int bottom = newVertices.second->score;
    while (true) {
        if (top != newVertices.first->word.length()) {
            possibleMax += top--;
        }
        if (bottom != newVertices.first->word.length()) {
            possibleMax += bottom--;
        }
        if (top == newVertices.first->word.length() && bottom == top) {
            if (possibleMax + 1 > longestLadder.score) {
                break;
            } else {
                return;
            }
        }
    }
    visitedVertices.insert(newVertices.first);
    currentLadder.score += newVertices.first->score;
    currentLadder.vertices.push_front(newVertices.first);
    // Checks if only one element in ladder.
    if (newVertices.first != newVertices.second) {
        visitedVertices.insert(newVertices.second);
        currentLadder.score += newVertices.second->score;
        currentLadder.vertices.push_back(newVertices.second);
    }
    if (currentLadder.score > longestLadder.score) longestLadder = currentLadder;
    
    for (vector<vertex *>::iterator headItr =
         wordGraph.verticesAndLinks[currentLadder.vertices.front()].begin();
         headItr != wordGraph.verticesAndLinks[currentLadder.vertices.front()].end(); ++headItr) {
        
        for (vector<vertex *>::iterator tailItr =
             wordGraph.verticesAndLinks[currentLadder.vertices.back()].begin();
             tailItr != wordGraph.verticesAndLinks[currentLadder.vertices.back()].end(); ++tailItr) {
            
            if (*headItr != *tailItr &&
                !(visitedVertices.count(*headItr) || visitedVertices.count(*tailItr))) {
                
                searchFromVertex(wordGraph, currentLadder, longestLadder, visitedVertices, make_pair(*headItr, *tailItr));
            }
        }
    }
}

// word score comparison for sorting the input word list.
// Comparison is backwards so that words are sorted from
// smallest to largest so that at every pass through map
// during graph construction, all adjacient words to the
// current are already within the map.
// Note, actually slows things down for website test inputs. 
bool computeScoreComparison(string lhs, string rhs) {
    return computeWordScore(lhs) < computeWordScore(rhs);
}

// Iterates through the array of words and creates all vertices
// Creates a vertex and adds it to the graph.
// Iterates through the graph and if a vertex's word has a distance of 1
// from the current vertex's word and a different scrabble score, it is
// added to the current vertex's set of connected nodes.
graph makeGraph(vector<string> & sortedWords) {
    graph result;
    sort(sortedWords.begin(), sortedWords.end(), computeScoreComparison);
    
    for (int i = 0; i < sortedWords.size(); i++) {
        vertex *currVert = new vertex;
        currVert->word = sortedWords[i];
        currVert->score = computeWordScore(currVert->word);
        vector<vertex *> holder;
        
        for (map<vertex *, vector<vertex *> >::iterator tempItr = result.verticesAndLinks.begin();
             tempItr != result.verticesAndLinks.end(); tempItr++) {
            
            pair<vertex *, vector<vertex *> > tempItem = *tempItr;
            if (isAdjacientVertex(tempItem.first->word, currVert->word) &&
                currVert->score > tempItem.first->score) {
                holder.push_back(tempItem.first);
            }
        }
        result.verticesAndLinks[currVert] = holder;
    }
    
    if (isDebugging) {
        for (map<vertex *, vector<vertex *> >::iterator vertItr = result.verticesAndLinks.begin();
             vertItr != result.verticesAndLinks.end(); ++vertItr) {
            pair<vertex *, vector<vertex *> > currItem = *vertItr;
            cout << "------" << endl;
            cout << currItem.first->word << endl;
            cout << currItem.second.size() << endl;
            for (int i = 0; i < currItem.second.size(); i++) {
                cout << currItem.second[i]->word << " " << currItem.second[i]->score << endl;
            }
        }
    }
    
    return result;
}

// Returns true if strings are 1 perm apart.
bool isAdjacientVertex(string str1, string str2) {
    int distance = 0;
    for (int i = 0; i < str1.length(); i++) {
        if (str1[i] != str2[i]) distance += 1;
    }
    return distance == 1 ? true : false;
}

// Get all inputs storing each word as an entry in a set.
// Only words of length 'K' are added to the set.
// Words if uppercased just incase the input isn't.
vector<string> getInput() {
    vector<string> result;
    int lengthOfWordsToConsider;
    int numDictWords;
    cin >> lengthOfWordsToConsider;
    cin >> numDictWords;
    
    for (int i = 0; i < numDictWords; i++) {
        string tempInput = "";
        cin >> tempInput;
        if (tempInput.length() == lengthOfWordsToConsider) {
            if (tempInput[0] > 'Z') {
                for (int i = 0; i < tempInput.length(); i++) {
                    tempInput[i] = toupper(tempInput[i]);
                }
            }
            result.push_back(tempInput);
        }
    }
    return result;
}

// Helper method prints out a ladder
void printLadder(deque<vertex *> ladder) {
    for (int i = 0; i < ladder.size(); i++) {
        cout << ladder[i]->word << " " << ladder[i]->score << endl;
    }
    cout << endl;
}

// Compute the total scrabble score of a ladder
int scrabbleScore(deque<vertex *> & ladderToCompute) {
    int score = 0;
    if (ladderToCompute.size() == 0) return 0;
    for (int i = 0; i < ladderToCompute.size(); i++) {
        score += ladderToCompute[i]->score;
    }
    return score;
}

// compute the scrabble score of a single string.
int computeWordScore(string & wordToCheck) {
    int score = 0;
    for (int i = 0; i < wordToCheck.length(); i++) {
        switch (wordToCheck[i]) {
            case 'A': case 'E': case 'I': case 'L': case 'N': case 'O': case 'R': case 'S': case 'T': case 'U':
                score += 1;
                break;
            case 'D': case 'G':
                score += 2;
                break;
            case 'B': case 'C': case 'M': case 'P':
                score += 3;
                break;
            case 'F': case 'H': case 'V': case 'W': case 'Y':
                score += 4;
                break;
            case 'K':
                score += 5;
                break;
            case 'J': case 'X':
                score += 8;
                break;
            case 'Q': case 'Z':
                score += 10;
                break;
            default:
                break;
        }
    }
    return score;
}