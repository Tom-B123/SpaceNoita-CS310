#include "app.h"
#include "materials.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"

#include <iostream>
int load_materials(float* colours,Material* material_data) {
    // Read the entire file into a string
    std::string materials_location = find_shader_file("../cpp/materials.json");
    std::string rules_location = find_shader_file("../cpp/rules.json");

    // std::cout << location << std::endl;

    FILE* materials_file = fopen(materials_location.c_str(), "r");

    if (materials_file == nullptr) {
        std::cerr << "Failed to locate materials.json!";
        return -1;
    }

    // Use a FileReadStream to read the data from the file
    char readBuffer[65536];
    rapidjson::FileReadStream materials_json(materials_file, readBuffer,
                                 sizeof(readBuffer));

    // Parse the JSON data using a Document object
    rapidjson::Document materials_document;
    materials_document.ParseStream(materials_json);

    fclose(materials_file);

    FILE* rules_file = fopen(rules_location.c_str(), "r");

    if (rules_file == nullptr) {
        std::cerr << "Failed to locate rules.json!";
        return -1;
    }

    char readBuffer2[65536];
    rapidjson::FileReadStream rules_json(rules_file, readBuffer2,
                                 sizeof(readBuffer2));

    // Parse the JSON data using a Document object
    rapidjson::Document rules_document;
    rules_document.ParseStream(rules_json);

    // Close the files
    fclose(rules_file);

    // Loop over all json elements
    for (rapidjson::Value::ConstMemberIterator itr = 
            materials_document.MemberBegin();
            itr != materials_document.MemberEnd(); ++itr)
    {
        // Get each key, this is the material code as an ascii character
        char material_code = itr->name.GetString()[0];
        Material material;

        // Store the material data inside a material struct, with fixed
        // size for the name, colour and state strings.
        for (int i = 0; i < 128; i++) {
            material.name[i]=itr->value["name"].GetString()[i];
        } 
        for (int i = 0; i < 8; i++) {
            material.colour[i]=itr->value["colour"].GetString()[i];
        }

        std::string state_string = itr->value["state"].GetString();

        material.state = state_from_string(state_string);

        material.density = itr->value["density"].GetInt();

        material_data[material_code] = material;

        std::cout << material.name << ": " << material.colour << std::endl;
    }

    for (int material = 0; material < 256; material++) {

        char* colour = material_data[material].colour;

        // Convert the raw #RRGGBB- code into 3 integers from 0-256
        int r = hex2(colour,1);
        int g = hex2(colour,3);
        int b = hex2(colour,5);

        if (r < 0 || g < 0 || b < 0) { continue; }
        
        colours[3 * material + 0] = r/256.0;
        colours[3 * material + 1] = g/256.0;
        colours[3 * material + 2] = b/256.0;
    }

    return 0;
}
