#include "Config.hh"

NAMESPACE_BEGIN

void Config::parseConfig(){
    bool result = true;


    YAML::Node appcontext = rootNode["generalinfo"];
    appcontext = appcontext["debug"];
    if(appcontext){
        cfgappcontext.consoleoutput = appcontext["console"].as<bool>(cfgappcontext.consoleoutput);
        cfgappcontext.fileoutput = appcontext["logfile"].as<bool>(cfgappcontext.fileoutput);
        cfgappcontext.fileoutputappend = appcontext["logfileappend"].as<bool>(cfgappcontext.fileoutputappend);
    }


    YAML::Node rendersystem = rootNode["rendersystem"];
    if(rendersystem){
        this->cfgrendersystem.screenheight = rendersystem["screenheight"].as<int>(cfgrendersystem.screenheight);
        cfgrendersystem.screenwidth = rendersystem["screenwidth"].as<int>(cfgrendersystem.screenwidth);
        cfgrendersystem.screenmode = rendersystem["screenmode"].as<std::string>(cfgrendersystem.screenmode);
        cfgrendersystem.vsync = rendersystem["vsync"].as<bool>(cfgrendersystem.vsync);
        cfgrendersystem.msaa = rendersystem["msaa"].as<int>(cfgrendersystem.msaa);


    }

    YAML::Node assetsystem = rootNode["assetsystem"];
    if(assetsystem){
        cfgassetsystem.assetpath = assetsystem["rootdirectory"].as<std::string>(cfgassetsystem.assetpath);
    }



}

NAMESPACE_END