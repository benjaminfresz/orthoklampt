#pragma once

#include <tinyxml.h>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

inline bool CheckNodeName(TiXmlElement *node, const char* name)
{
  if(0!=strcmp(node->Value(),name)) {
    std::cout << "Not a " << name <<  " file" << std::endl;
    return false;
  }
  return true;
}
inline TiXmlElement* GetRootNodeFromDocument(TiXmlDocument& doc)
{
  if(doc.LoadFile()){
    TiXmlElement *root = doc.RootElement();
    if(root){
      return root;
    }
  }else{
    std::cout << doc.ErrorDesc() << std::endl;
  }
  return NULL;
}

inline TiXmlElement* FindSubNode(TiXmlElement* node, const char *name){
  if(!node) return NULL;
  TiXmlElement* e=node->FirstChildElement();
  while(e != NULL) 
  {
    if(0==strcmp(e->Value(),name)) return e; 
    e = e->NextSiblingElement();
  }
  return NULL;
}

inline TiXmlElement* FindFirstSubNode(TiXmlElement* node, const char *name){
  return FindSubNode(node, name);
}

inline TiXmlElement* FindNextSiblingNode(TiXmlElement* node, const char *name){
  while(node != NULL) 
  {
    node = node->NextSiblingElement();
    if(node!=NULL){
      if(0==strcmp(node->Value(),name)) return node; 
    }
  }
  return NULL;
}

inline bool ExistStreamAttribute(TiXmlElement* node, const char *name){
  if(!node) return false;
  const char *na = node->Attribute(name);
  if(na){
    return true;
  }else{
    return false;
  }
}

inline stringstream GetStreamAttribute(TiXmlElement* node, const char *name){

  if(!node) return stringstream ("NONE");
  const char *na = node->Attribute(name);
  if(na){
    return stringstream (na);
  }else{
    return stringstream ("NONE");
  }
}
inline stringstream GetStreamAttributeConfig(TiXmlElement* node, const char *name){

  if(!node) return stringstream ("NONE");
  const char *na = node->Attribute(name);
  if(na){
    //safety check for config
//  int n;
//  in >> n;
//  if(!in) return in;
//  if(n != v.n)
//    v.resize(n);
//  for(int i=0; i<v.n; i++)
//    in >> v[i];
//  return in;
//}

    return stringstream (na);
  }else{
    return stringstream ("NONE");
  }
}

inline stringstream GetStreamText(TiXmlElement* node){

  if(!node) return stringstream ("NONE");
  const char *na = node->GetText();
  if(na){
    return stringstream (na);
  }else{
    return stringstream ("NONE");
  }
}

template<typename T> 
inline stringstream GetStreamTextDefault(TiXmlElement* node, T default_value){
  std::stringstream ss;
  ss = GetStreamText(node);
  if(ss.str() != "NONE"){
    return ss;
  }else{
    std::stringstream def;
    def << default_value;
    return def;
  }
}
template<typename T> 
inline stringstream GetStreamAttributeDefault(TiXmlElement *node, const char *name, T default_value){
  if(ExistStreamAttribute(node, name)){
    return GetStreamAttribute(node, name);
  }
  stringstream ss;
  ss << default_value;
  return ss;
}
