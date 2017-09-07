#include "main.h"

int main(int argc, char **argv)
{
  Engine::Context ctx;

  //Do command line arguments
  int exit = processArgs(argc, argv, ctx);

  if(exit != -1) {
    return exit;
  }

  // Start an engine and run it then cleanup after
  Engine *engine = new Engine(ctx);
  if(!engine->Initialize())
  {
    printf("The engine failed to start.\n");
    delete engine;
    engine = NULL;
    return 1;
  }
  engine->Run();
  delete engine;
  engine = NULL;
  return 0;
}

int processArgs(int argc, char** argv, Engine::Context& ctx) {
  int argIndex = 1;
  int error;

  //If no arguments, tell user to look at help menu
  if(argc == 1) {
    std::cout << "No arguments found" << std::endl
              << "Please run with --help for command reference" << std::endl;
    return 1;
  }

  //Loop through arguments and process
  for(; argIndex < argc; argIndex++) {
    //HELP MENU
    if(!strcmp(argv[argIndex],"--help")) {
      helpMenu();
      return 0;
    } 
    //VERTEX SHADER FILE
    else if (!strcmp(argv[argIndex], "-v") || !strcmp(argv[argIndex], "--vertex")) {
      //Make certain this isn't the last argument
      //The next argument should be the filename
      if(++argIndex < argc) {
        error = loadShader(argv[argIndex], ctx.vertex);
        if(error != -1) return error;
      } else {
        std::cout << "Expected argument for -v";
        return 1;
      }
      continue;
    } 
    //FRAGMENT SHADER FILE
    else if (!strcmp(argv[argIndex], "-f") || !strcmp(argv[argIndex], "--fragment")) {
      //Make certain this isn't the last argument
      //The next argument should be the filename
      if(++argIndex < argc) {
        error = loadShader(argv[argIndex], ctx.fragment);
        if(error != -1) return error;
      } else {
        std::cout << "Expected argument for -f";
        return 1;
      }
      continue;
    }

    std::cout << "Unexpected argument: " << argv[argIndex] << std::endl;
  }

  if(ctx.vertex == "") {
    std::cout << "No -v option included" << std::endl;
    return 1;
  }
  if(ctx.fragment == "") {
    std::cout << "No -f option included" << std::endl;
    return 1;
  }

  return -1;
}

int loadShader(const std::string& filename, std::string& shader) {
  std::ifstream shaderFile(filename);
    
  if(!shaderFile.is_open()) {
      std::cerr << "Could not find shader file: " << filename << std::endl;
      return 1;
  }

  shader = std::string(std::istreambuf_iterator<char>(shaderFile), {});

  return -1;
}

void helpMenu() {
  std::cout << "Command Usage:" << std::endl << std::endl
            << "    " << PROGRAM_NAME << " --help" << std::endl
            << "    " << PROGRAM_NAME << " [options] <-f filename> <-v filename>" << std::endl << std::endl
            << "Options" << std::endl
            << "    --help                       Show help menu and command usage" << std::endl
            << "    -v, --vertex <filename>      Specify where to load vertex shader from" << std::endl
            << "    -f, --fragment <filename>    Specify where to load fragment shader from" << std::endl;
}