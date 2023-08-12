
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <cassert>  //assert
#include <utility>
#include <cmath>
#include <algorithm>
#include "csvstream.h"
using namespace std;

class Classifier {
  public:
    Classifier() {
      num_post = 0;
      num_post1 = 0;
    }
    
    bool error_checking1(int argc, char* argv[]) {
      if(argc != 3 && argc != 4) {
        cout << "Usage: main.exe TRAIN_FILE TEST_FILE [--debug]" << endl;
        return true;
      }
      if(argc == 4) {
        if(strcmp(argv[3], "--debug") != 0) {
          cout << "Usage: main.exe TRAIN_FILE TEST_FILE [--debug]" << endl;
          return true;
        }
      }
      return false;
    }

    bool error_checking2(char* argv[]) {
      try {
        csvstream train(argv[1]);
      }
      catch(const csvstream_exception &e) {
        cout << "Error opening file: " << argv[1] << endl;
        return true;
      }
      try {
        csvstream train(argv[2]);
      }
      catch(const csvstream_exception &e) {
        cout << "Error opening file: " << argv[2] << endl;
        return true;
      }
      return false;
    }

    void training(char* argv[]) {
      csvstream train(argv[1]);
      map<string, string> row;
      while(train >> row) {
        string tag = row["tag"];
        tag_freq[tag]++;
        istringstream content(row["content"]);
        set<string> words;
        string word;
        while (content >> word) {
          words.insert(word);
          uniq_vocabs.insert(word);
        }
        for(auto it = words.begin(); it != words.end(); it++) {
          word_freq[*it]++;
          tag_word_freq[tag][*it]++;
        }
        num_post++;
        num_post1++;
      }
      vocab_size = uniq_vocabs.size();
    }

  void predict(char* argv[]) {
    cout << "test data:";
    csvstream predict(argv[2]);
    map<string,string> row;
    int testpost = 0;
    while(predict >> row) {
      testpost++;
      string tag = row["tag"];
      string contentt = row["content"];
      istringstream content(row["content"]);
      set<string> uwords;
      string word;
      max = -9999999;
      string predicted;
      while (content >> word) {
        uwords.insert(word);
      }
      for (auto it = tag_freq.begin(); 
           it != tag_freq.end(); it++) {
        total = 0;
        
        //it->first is calculator and euchre
        for (auto str: uwords) {
          total += log_likelihood(it->first, str);
        }
        total += log_prior(it->first);
        // cout << total << "FUCK";
        if (total > max) {
          max = total;
          predicted = it->first;
        }
      }
      cout << '\n';
      if (tag == predicted) {
        correct_counter++;
      }
      cout << "  correct = " << tag << ", predicted = "
           << predicted << ", log-probability score = " 
           << max << '\n'
           << "  content = " << contentt << '\n';

    }
    cout << "\nperformance: " << correct_counter
         << " / " << testpost << " posts predicted correctly\n";
  }

    void print_debug(char* argv[]) {
      csvstream train(argv[1]);
      csvstream predict(argv[2]);
      cout << "training data:\n";
      map<string, string> row;
      map<string, string> row1;
      while (train >> row) {
        string tag = row["tag"];
        string word = row["content"];
        cout << "  label = " << tag << ", content = "
             << word << "\n";
      }
      cout << "trained on " << num_post1 << " examples\n"
           << "vocabulary size = " << vocab_size << "\n\n"
           << "classes:\n";
      
      auto it = tag_freq.begin();
      while (predict >> row1) {
        string label = row["tag"];
        string word = row["content"];

        while (it != tag_freq.end()) {
          cout << "  " << it->first << ", " << it->second
              << " examples, log-prior = " << log_prior(it->first) << '\n';
          it++;
        }
      }

      cout << "classifier parameters:\n";
      print_loglikelihood(argv);
      
    }
    void print_loglikelihood(char* argv[]){
      csvstream train(argv[2]);
      map<string, string> row;
      auto it = tag_freq.begin();
        while (it != tag_freq.end()) {
          //update data for given label
          auto it2 = tag_word_freq[it->first].begin();
          while (it2 != tag_word_freq[it->first].end()){
            //update data for given word
            cout << "  " << it->first << ":" << it2->first
                 << ", count = " << it2->second 
                 << ", log-likelihood = " << log_likelihood(it->first ,it2->first)
                 << '\n';
            it2++;
          }
          it++;
        }
    }
    
    void p_opening() {
      cout << "trained on " << num_post1 << " examples\n\n";
    }

    double log_prior(string label) {
      return log(tag_freq[label] / num_post);
    }

    double log_likelihood(string label, string word) {
      double likelihood = 0;
      if(tag_word_freq[label][word] != 0) {
        likelihood = log((0.0 + tag_word_freq[label][word]) / tag_freq[label]);
      }
      else if (word_freq[word] != 0) {
        likelihood = log(word_freq[word] / num_post);
        // cout << likelihood;
      }
      else {
        likelihood = log(1 / num_post);
        // cout << "not seen";
      }

      return likelihood;
    }

  private:
  map<string, int> tag_freq;
  set<string> uniq_vocabs; //useful for content(count words only once)
  map<string, int> word_freq;
  map<string, map<string,int>> tag_word_freq;
  double num_post;
  int num_post1;
  int vocab_size, correct_counter = 0;
  double total, max;

};

int main(int argc, char* argv[]) {
  cout.precision(3);
  Classifier c;
  if(c.error_checking1(argc, argv) || 
     c.error_checking2(argv)) {
    return 1;
  } 
  c.training(argv);
  if(argc == 4) {
    if(strcmp(argv[3], "--debug") == 0) {
    c.print_debug(argv);
    c.predict(argv);
    }
  }
  else {
    c.p_opening();
    c.predict(argv);
  }

  return 0;
}
