// $Id: commands.cpp,v 1.18 2019-10-08 13:55:31-07 - - $

#include "commands.h"
#include "debug.h"
#include "map"
#include <stack>

command_hash cmd_hash {
   {"#"     , fn_comment},
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },
   
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   auto result = cmd_hash.find (cmd);
   if (cmd.at(0) == '#') { 
      result = cmd_hash.find("#");
   }
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int status = exec::status();
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}

void fn_comment (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

string clean_cd_to_command (inode_state& state, const wordvec& words,
   bool do_extra) {

   if (words.size() <= 1) { return "";}
   wordvec list_of_words = split(words.at(1),"/");
   int size = do_extra ? list_of_words.size() : list_of_words.size()-1;
   for (int word_iterator = 0; word_iterator<size;
      word_iterator++) {
         wordvec deeper_cd_command;
         deeper_cd_command.insert(deeper_cd_command.end(), "cd");
         deeper_cd_command.insert(deeper_cd_command.end(),
            list_of_words.at(word_iterator));
         fn_cd(state, deeper_cd_command);
   }
   return list_of_words.at(list_of_words.size()-1);
}

void cd_back_command (inode_state& state, const wordvec& words,
   bool do_extra) {

  if (words.size() <= 1) { return;}
   wordvec list_of_words = split(words.at(1),"/");
   int size = do_extra ? list_of_words.size() : list_of_words.size()-1;
   for (int word_iterator = 0; word_iterator<size;
      word_iterator++) {
         wordvec deeper_cd_command;
         deeper_cd_command.insert(deeper_cd_command.end(), "cd");
         deeper_cd_command.insert(deeper_cd_command.end(), "..");
         fn_cd(state, deeper_cd_command);
   }
   return;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   wordvec origword = words;

   string s_target = clean_cd_to_command(state, words, false);

   cout << state.get_cwd_ptr()->get_base_file_ptr()->get_dirents().
      find(s_target)->second->get_base_file_ptr()->readfile()<<endl;

   cd_back_command(state, origword, false);
}

void fn_cd (inode_state& state, const wordvec& words){
   if (words.size() > 1) {
      if (split(words.at(1),"/").size()>1) {
         clean_cd_to_command(state, words, true);
      } else {
         map<string,inode_ptr> the_dirents = state.get_cwd_ptr()->
         get_base_file_ptr()->get_dirents();
         the_dirents.find(words.at(1));
         state.set_cwd(the_dirents.find(words.at(1))->second);
      }
   } else {
      state.set_cwd(state.get_root_ptr());
   }
   //TODO(me) maybe it isn't words at 1
   //
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   wordvec origword = words;
   bool isroot = false;
   if (words.size()>1 && words.at(1) == "/") {
      isroot = true;
   }

   map<string,inode_ptr> the_dirents;
   if (!isroot) {
      clean_cd_to_command(state, words, true);
      the_dirents = state.get_cwd_ptr()->
         get_base_file_ptr()->get_dirents();
   } else {
      the_dirents = state.get_root_ptr()->
         get_base_file_ptr()->get_dirents();
   }

   if (!isroot) {
      string ls_pwd = get_pwd(state,words).append(":");
      cout<<ls_pwd<<endl;
   } else { cout<<"/:"<<endl;}

   for (auto pair : the_dirents) {
      string name = pair.first;
      if (pair.second->get_base_file_ptr()->get_identity() ==
         file_type::DIRECTORY_TYPE && name != "." && name != "..") {
         name.append("/");
      }
      cout<<"\t"<<pair.second->get_inode_nr()<<"\t"<<pair.second
         ->get_base_file_ptr()->size()<<" "<<name<<endl;
   }

   if (!isroot){
      cd_back_command(state, origword, true);
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   wordvec origword = words;
   bool isroot = false;
   if (words.size()>1 && words.at(1) == "/") {
      isroot = true;
   }

   map<string,inode_ptr> the_dirents;
   if (!isroot) {
      clean_cd_to_command(state, words, true);
      the_dirents = state.get_cwd_ptr()->
         get_base_file_ptr()->get_dirents();
   } else {
      the_dirents = state.get_root_ptr()->
         get_base_file_ptr()->get_dirents();
   }

   if (!isroot) {
      string ls_pwd = get_pwd(state,words).append(":");
      cout<<ls_pwd<<endl;
   } else { cout<<"/:"<<endl;}

   for (auto pair : the_dirents) {
      string name = pair.first;
      if (pair.second->get_base_file_ptr()->get_identity() ==
         file_type::DIRECTORY_TYPE && name != "." && name != "..") {
         name.append("/");
      }
      cout<<"\t"<<pair.second->get_inode_nr()<<"\t"<<pair.second
         ->get_base_file_ptr()->size()<<" "<<name<<endl;

      if (pair.second->get_base_file_ptr()->get_identity() ==
         file_type::DIRECTORY_TYPE && name != "." && name != "..") {
         wordvec deeper_cd_command;
         deeper_cd_command.insert(deeper_cd_command.end(),"cd");
         deeper_cd_command.insert(deeper_cd_command.end(),pair.first);
         fn_cd(state,deeper_cd_command);
         wordvec deeper_lsr_command;
         deeper_lsr_command.insert(deeper_lsr_command.end(),"lsr");
         fn_lsr(state,deeper_lsr_command);
         wordvec shallower_cd_command;
         shallower_cd_command.insert(shallower_cd_command.end(),"cd");
         shallower_cd_command.insert(shallower_cd_command.end(),"..");
         fn_cd(state,shallower_cd_command);
      }

   }
   if (!isroot) {
      cd_back_command(state, origword, true);
   }
}

void fn_make (inode_state& state, const wordvec& words){
   wordvec origword = words;
   string s_target = clean_cd_to_command(state, words, false);
   wordvec the_content(words.begin()+2, words.end());
   state.get_cwd_ptr()->get_base_file_ptr()->mkfile(s_target,
      the_content);
   cd_back_command(state, origword, false);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   wordvec origword = words;
   string s_target = clean_cd_to_command(state, words, false);
   state.get_cwd_ptr()->get_base_file_ptr()->mkdir(s_target);
   cd_back_command(state, origword, false);
}

void fn_prompt (inode_state& state, const wordvec& words){
   state.set_prompt_(append_from(1,words));
}

string get_pwd (inode_state& state, const wordvec&) {
   wordvec the_pwd_vec;
   inode_ptr the_inode = state.get_cwd_ptr();

   while (the_inode->get_inode_nr() != state.get_root_ptr()
      ->get_inode_nr()){

      the_pwd_vec.insert(the_pwd_vec.end(),the_inode
      ->get_base_file_ptr()->get_name());
      the_inode = the_inode->get_parent().lock();
   }

   string final_pwd = "/";
   for (int pwd_vec_iterator = the_pwd_vec.size()-1;
      pwd_vec_iterator >= 0; pwd_vec_iterator--) {
      if (pwd_vec_iterator != static_cast<int>(the_pwd_vec.size()-1)) {
         final_pwd.append("/");
      }
      final_pwd.append(the_pwd_vec.at(pwd_vec_iterator));
   }
   return final_pwd;
}

void fn_pwd (inode_state& state, const wordvec& words){
   cout<<get_pwd(state,words)<<endl;
}

void fn_rm (inode_state& state, const wordvec& words){
   wordvec origword = words;
   string s_target = clean_cd_to_command(state, words, false);
   inode_ptr target = state.get_cwd_ptr()->get_base_file_ptr()
   ->get_dirents().find(s_target)->second;

   if (target->get_base_file_ptr()->get_identity() == 
      file_type::PLAIN_TYPE ||target->get_base_file_ptr()->
      get_dirents().size() <= 2) {

      state.get_cwd_ptr()->get_base_file_ptr()->remove(s_target);
   } else {
      cout<< "you cant do that!"<< endl;   
   }
   cd_back_command(state, origword, false);
}

void fn_rmr (inode_state& state, const wordvec& words){
   wordvec origword = words;
   string s_target = clean_cd_to_command(state, words, false);
   state.get_cwd_ptr()->get_base_file_ptr()->remove(s_target);
   cd_back_command(state, origword, false);
}

