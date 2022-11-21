#include "App.h"
#include "Window.h"
#include "Input.h"
#include "Render.h"
#include "Textures.h"
#include "Audio.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Map.h"
#include "Physics.h"
#include "Fonts.h"

#include "Defs.h"
#include "Log.h"

#include <iostream>
#include <sstream>

// Constructor
App::App(int argc, char* args[]) : argc(argc), args(args)
{
	frames = 0;

	input = new Input();
	win = new Window();
	render = new Render();
	tex = new Textures();
	audio = new Audio();
	physics = new Physics();
	scene = new Scene();
	entityManager = new EntityManager();
	map = new Map();
	fonts = new Fonts();

	// Ordered for awake / Start / Update
	// Reverse order of CleanUp
	AddModule(input);
	AddModule(win);
	AddModule(tex);
	AddModule(audio);
	AddModule(physics);
	AddModule(scene);
	AddModule(entityManager);
	AddModule(map);
	AddModule(fonts);

	// Render last to swap buffer
	AddModule(render);
}

// Destructor
App::~App()
{
	// Release modules
	ListItem<Module*>* item = modules.end;

	while (item)
	{
		RELEASE(item->data)
		item = item->prev;
	}

	modules.Clear();
}

void App::AddModule(Module* module)
{
	module->Init();
	modules.Add(module);
}

// Called before render is available
bool App::Awake()
{
	if(!LoadConfig()) return false;

	title = configNode.child("app").child("title").child_value();

	ListItem<Module*>*item = modules.start;

	while(item)
	{
		pugi::xml_node node = configNode.child(item->data->name.GetString());
		if(!item->data->Awake(node)) return false;
		item = item->next;
	}

	return true;
}

// Called before the first frame
bool App::Start()
{
	ListItem<Module*>* item = modules.start;

	while (item)
	{
		if(!item->data->Start()) return false;
		item = item->next;
	}

	return true;
}

// Called each loop iteration
bool App::Update()
{
	PrepareUpdate();

	if(input->GetWindowEvent(WE_QUIT)) return false;
	if(!PreUpdate()) return false;
	if(!DoUpdate()) return false;
	if(!PostUpdate()) return false;

	FinishUpdate();

	return true;
}

// Load config from XML file
bool App::LoadConfig()
{
	pugi::xml_parse_result parseResult = configFile.load_file("config.xml", pugi::parse_default | pugi::parse_comments);

	if(parseResult) configNode = configFile.child("config");
	else LOG("Error in App::LoadConfig(): %s", parseResult.description());

	return parseResult;
}

// ---------------------------------------------
void App::PrepareUpdate()
{
}

// ---------------------------------------------
void App::FinishUpdate()
{
	if (loadGameRequested) LoadFromFile();
	if (saveGameRequested) SaveToFile();
}

// Call modules before each loop iteration
bool App::PreUpdate()
{
	for (ListItem<Module*>* item = modules.start; item; item = item->next)
	{
		Module* pModule = item->data;
		if(!pModule->active) continue;
		if(!pModule->PreUpdate()) return false;
	}
	return true;
}

// Call modules on each loop iteration
bool App::DoUpdate()
{
	for(ListItem<Module*>* item = modules.start; item; item = item->next)
	{
		Module* pModule = item->data;
		if(!pModule->active) continue;
		if(!pModule->Update(dt)) return false;
	}
	return true;
}

// Call modules after each loop iteration
bool App::PostUpdate()
{
	for(ListItem<Module *> *item = modules.start; item; item = item->next)
	{
		Module* pModule = item->data;
		if(!pModule->active) continue;
		if(!pModule->PostUpdate()) return false;
	}

	if(input->GetKey(SDL_SCANCODE_P) == KEY_DOWN)
	{
		return PauseGame();
	}
	return true;
}

// Called before quitting
bool App::CleanUp()
{
	ListItem<Module*>* item = modules.end;

	while (item)
	{
		if(!item->data->CleanUp()) return false;
		item = item->prev;
	}
	return true;
}

// ---------------------------------------
int App::GetArgc() const
{
	return argc;
}

// ---------------------------------------
const char* App::GetArgv(int index) const
{
	if (index < argc)
		return args[index];
	else
		return nullptr;
}

// ---------------------------------------
const char* App::GetTitle() const
{
	return title.GetString();
}

// ---------------------------------------
const char* App::GetOrganization() const
{
	return organization.GetString();
}

uint App::GetLevelNumber() const
{
	return levelNumber;
}

void App::LoadGameRequest()
{
	// NOTE: We should check if SAVE_STATE_FILENAME actually exist
	loadGameRequested = true;
}

// ---------------------------------------
void App::SaveGameRequest() 
{
	// NOTE: We should check if SAVE_STATE_FILENAME actually exist and... should we overwriten
	saveGameRequested = true;
}


bool App::LoadFromFile()
{
	bool ret = true;

	pugi::xml_document gameStateFile;
	pugi::xml_parse_result result = gameStateFile.load_file("save_game.xml");

	if (result == NULL)
	{
		LOG("Could not load xml file savegame.xml. pugi error: %s", result.description());
		return false;
	}
	
	ListItem<Module*>* item = modules.start;

	while (item && ret)
	{
		const char* itemName = item->data->name.GetString();
		pugi::xml_node loadStateOnFile = gameStateFile.child("save_state");

		if(!item->data->LoadState(loadStateOnFile.child(itemName))) return false;
		item = item->next;
	}


	loadGameRequested = false;

	return true;
}

bool App::SaveToFile() 
{
	auto* saveDoc = new pugi::xml_document();
	pugi::xml_node saveStateNode = saveDoc->append_child("save_state");

	ListItem<Module*>* item = modules.start;


	while (item)
	{
		const char *itemName = item->data->name.GetString();

		if(!item->data->SaveState(saveStateNode.append_child(itemName))) return false;
		item = item->next;
	}

	if(!saveDoc->save_file("save_game.xml")) return false;

	saveGameRequested = false;

	return true;
}

bool App::PauseGame() const
{
	physics->ToggleStep();
	while(input->GetKey(SDL_SCANCODE_P) == KEY_DOWN || input->GetKey(SDL_SCANCODE_P) == KEY_REPEAT)
	{
		input->PreUpdate();
		if(app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) return false;
	}
	while(input->GetKey(SDL_SCANCODE_P) == KEY_IDLE || input->GetKey(SDL_SCANCODE_P) == KEY_UP)
	{
		input->PreUpdate();
		if(app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) return false;
	}
	physics->ToggleStep();
	return true;
}

bool App::SaveToConfig(std::string const &moduleName, std::string const &node, std::string const &attribute, std::string const &value) const
{
	if(configNode.child(moduleName.c_str()).child(node.c_str()).attribute(attribute.c_str()))
	{
		pugi::xml_attribute attr = configNode.child(moduleName.c_str()).child(node.c_str()).attribute(attribute.c_str());
		attr.set_value(value.c_str());

		if(!configFile.save_file("config.xml"))
		{
			LOG("Can't save config");
			return false;
		}
	}

	return true;
}
