/* this is a CMake loadable command to wrap vtk objects into Java */

#include "cmCPluginAPI.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct 
{
  char *LibraryName;
  int NumberWrapped;
  char **SourceFiles;
} cmVTKWrapJavaData;

/* do almost everything in the initial pass */
static int InitialPass(void *inf, void *mf, int argc, char *argv[])
{
  cmLoadedCommandInfo *info = (cmLoadedCommandInfo *)inf;
  int i;
  int newArgc;
  char **newArgv;
  int numClasses = 0;
  char **classes = 0;
  int numWrapped = 0;
  cmVTKWrapJavaData *cdata = 
    (cmVTKWrapJavaData *)malloc(sizeof(cmVTKWrapJavaData));
  const char *cdir = info->CAPI->GetCurrentDirectory(mf);
  const char *def = 0;
  char *sourceListValue = 0;
  char *newName;
  void *cfile = 0;

  if(argc < 3 )
    {
    info->CAPI->SetError(info, "called with incorrect number of arguments");
    return 0;
    }
  
  info->CAPI->ExpandSourceListArguments(mf, argc, argv, 
                                        &newArgc, &newArgv, 2);
  
  /* Now check and see if the value has been stored in the cache */
  /* already, if so use that value and don't look for the program */
  if(!info->CAPI->IsOn(mf,"VTK_WRAP_JAVA"))
    {
    info->CAPI->FreeArguments(newArgc, newArgv);
    return 1;
    }

  /* keep the library name */
  cdata->LibraryName = strdup(newArgv[0]);

  /* was the list already populated */
  def = info->CAPI->GetDefinition(mf, newArgv[1]);
  sourceListValue = 
    (char *)malloc(info->CAPI->GetTotalArgumentSize(newArgc,newArgv)+
                   newArgc*12 + (def ? strlen(def) : 1));  
  if (def)
    {
    sprintf(sourceListValue,"%s",def);
    }

  /* get the classes for this lib */
  for(i = 2; i < newArgc; ++i)
    {   
    void *curr = info->CAPI->GetSource(mf,newArgv[i]);
    
    /* if we should wrap the class */
    if (!curr || 
        !info->CAPI->SourceFileGetPropertyAsBool(curr,"WRAP_EXCLUDE"))
      {
      void *file = info->CAPI->CreateSourceFile();
      char *srcName;
      char *hname;
      srcName = info->CAPI->GetFilenameWithoutExtension(newArgv[i]);
      if (curr)
        {
        int abst = info->CAPI->SourceFileGetPropertyAsBool(curr,"ABSTRACT");
        info->CAPI->SourceFileSetProperty(file,"ABSTRACT",
                                          (abst ? "1" : "0"));
        }
      classes[numClasses] = strdup(srcName);
      numClasses++;
      newName = (char *)malloc(strlen(srcName)+5);
      sprintf(newName,"%sJava",srcName);
      info->CAPI->SourceFileSetName2(file, newName, 
                                     info->CAPI->GetCurrentOutputDirectory(mf),
                                     "cxx",0);

      hname = (char *)malloc(strlen(cdir) + strlen(srcName) + 4);
      sprintf(hname,"%s/%s.h",cdir,srcName);
      /* add starting depends */
      info->CAPI->SourceFileAddDepend(file,hname);
      info->CAPI->AddSource(mf,file);
      free(hname);
      cdata->SourceFiles[numWrapped] = file;
      numWrapped++;
      strcat(sourceListValue,";");
      strcat(sourceListValue,newName);
      strcat(sourceListValue,".cxx");        
      free(newName);
      }
    }
  
  cdata->NumberWrapped = numWrapped;
  info->CAPI->SetClientData(info,cdata);

  info->CAPI->AddDefinition(mf, newArgv[1], sourceListValue);
  info->CAPI->FreeArguments(newArgc, newArgv);
  return 1;
}
  
  
static void FinalPass(void *inf, void *mf) 
{
  cmLoadedCommandInfo *info = (cmLoadedCommandInfo *)inf;
  /* get our client data from initial pass */
  cmVTKWrapJavaData *cdata = 
    (cmVTKWrapJavaData *)info->CAPI->GetClientData(info);
  
  /* first we add the rules for all the .h to Java.cxx files */
  const char *wjava = "${VTK_WRAP_JAVA_EXE}";
  const char *pjava = "${VTK_PARSE_JAVA_EXE}";
  const char *hints = info->CAPI->GetDefinition(mf,"VTK_WRAP_HINTS");
  const char *args[4];
  const char *depends[2];
  const char *depends2[2];
  int i;
  int numDepends, numArgs;
  const char *cdir = info->CAPI->GetCurrentDirectory(mf);
  const char *resultDirectory = "${VTK_JAVA_HOME}";
  
  /* wrap all the .h files */
  depends[0] = wjava;
  depends2[0] = pjava;
  numDepends = 1;
  if (hints)
    {
    depends[1] = hints;
    depends2[1] = hints;
    numDepends++;
    }
  for(i = 0; i < cdata->NumberWrapped; i++)
    {
    char *res;
    const char *srcName = info->CAPI->SourceFileGetSourceName(cdata->SourceFiles[i]);
    char *hname = (char *)malloc(strlen(cdir) + strlen(srcName) + 4);
    sprintf(hname,"%s/%s",cdir,srcName);
    hname[strlen(hname)-4]= '\0';
    strcat(hname,".h");
    args[0] = hname;
    numArgs = 1;
    if (hints)
      {
      args[1] = hints;
      numArgs++;
      }
    args[numArgs] = 
      (info->CAPI->SourceFileGetPropertyAsBool(cdata->SourceFiles[i],"ABSTRACT") ?"0" :"1");
    numArgs++;
    res = (char *)malloc(strlen(info->CAPI->GetCurrentOutputDirectory(mf)) + 
                         strlen(srcName) + 6);
    sprintf(res,"%s/%s.cxx",info->CAPI->GetCurrentOutputDirectory(mf),srcName);
    args[numArgs] = res;
    numArgs++;
    info->CAPI->AddCustomCommand(mf, args[0],
                       wjava, numArgs, args, numDepends, depends, 
                       1, &res, cdata->LibraryName);
    free(res);

    res = (char *)malloc(strlen(resultDirectory) + 
                         strlen(srcName) + 7);
    sprintf(res,"%s/%s.java",resultDirectory,srcName);
    args[numArgs-1] = res;
    info->CAPI->AddCustomCommand(mf, args[0],
                                 pjava, numArgs, args, numDepends, depends2, 
                                 1, &res, cdata->LibraryName);
    free(res);
    free(hname);
    }
}

static void Destructor(void *inf) 
{
  cmLoadedCommandInfo *info = (cmLoadedCommandInfo *)inf;
  /* get our client data from initial pass */
  cmVTKWrapJavaData *cdata = 
    (cmVTKWrapJavaData *)info->CAPI->GetClientData(info);
  if (cdata)
    {
    info->CAPI->FreeArguments(cdata->NumberWrapped, cdata->SourceFiles);
    free(cdata->LibraryName);
    free(cdata);
    }
}

static const char* GetTerseDocumentation() 
{
  return "Create Java Wrappers.";
}

static const char* GetFullDocumentation()
{
  return
    "VTK_WRAP_JAVA(resultingLibraryName SourceListName SourceLists ...)";
}

void CM_PLUGIN_EXPORT VTK_WRAP_JAVA2Init(cmLoadedCommandInfo *info)
{
  info->InitialPass = InitialPass;
  info->FinalPass = FinalPass;
  info->Destructor = Destructor;
  info->GetTerseDocumentation = GetTerseDocumentation;
  info->GetFullDocumentation = GetFullDocumentation;  
  info->m_Inherited = 0;
  info->Name = "VTK_WRAP_JAVA2";
}




