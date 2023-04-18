#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <string>
#include <iostream>

class ShaderProgram {
public:
    unsigned int ID;

    ShaderProgram() {
        ID = createShaderProgram();
    }

    void setUniform1i(const std::string& name, int val) {
        glProgramUniform1i(ID, getLocation(name), val);
    }

    void setUniform1f(const std::string& name, float val) {
        glProgramUniform1f(ID, getLocation(name), val);
    }

    void setUniform3fv(const std::string& name, glm::vec3 val) {
        glProgramUniform3fv(ID, getLocation(name), 1, glm::value_ptr(val));
    }

    void setUniformMatrix4fv(const std::string& name, glm::mat4 val) {
        glUniformMatrix4fv(getLocation(name), 1, GL_FALSE, glm::value_ptr(val));
    }
    unsigned int getLocation(const std::string& name) {
        return glGetUniformLocation(ID, name.c_str());
    }
    unsigned int createShaderProgram() {
        std::string VertexShaderCode = "\
        #version 400 core\n\
        layout(location = 0) in vec3 position;\n\
        layout(location = 1) in vec2 uv;\n\
        layout(location = 2) in vec3 normal;\n\
        out vec2 vertex_uv;\n\
        out vec3 vertex_pos;\n\
        out vec3 vertex_normal;\n\
        uniform float time;\n\
        void main() {\n\
            //time dependent uv to change uv coordinate over time\n\
            vertex_uv = 2.0*(uv- time * 0.08) ;\n\
            vertex_pos = position;\n\
            vertex_normal = normal;\n\
       }\n";
        std::string TCShaderCode="\
    #version 400 core\n\
    //specify the number of vertices which is 4 for quads\n\
    layout(vertices = 4) out;\n\
    uniform float TessLevelInner;\n\
    uniform float TessLevelOuter;\n\
    in vec2 vertex_uv[];\n\
    in vec3 vertex_pos[];\n\
    in vec3 vertex_normal[]; \n\
    out vec2 tcs_uv[];\n\
    out vec3 tcs_pos[];\n\
    out vec3 tcs_normal[];\n\
    void main() {\n\
        //defined vertex data(position,uv,normal) out to an array with gl_Invoation ID as index\n\
        tcs_pos[gl_InvocationID] = vertex_pos[gl_InvocationID];\n\
        tcs_uv[gl_InvocationID] = vertex_uv[gl_InvocationID];\n\
        tcs_normal[gl_InvocationID] = vertex_normal[gl_InvocationID];\n\
        if (gl_InvocationID == 0) {\n\
            //define the outer four edge tess level and the 2 inner one(quads have two)\n\
            gl_TessLevelOuter[0] = TessLevelOuter; \n\
            gl_TessLevelOuter[1] = TessLevelOuter;\n\
            gl_TessLevelOuter[2] = TessLevelOuter;\n\
            gl_TessLevelOuter[3] = TessLevelOuter; \n\
            gl_TessLevelInner[0] = TessLevelInner;\n\
            gl_TessLevelInner[1] = TessLevelInner;\n\
        }\n\
    }\n";
        std::string TEShaderCode = "\
        #version 400 core\n\
        //abstract patch type quad\n\
        layout(quads, equal_spacing) in;\n\
        in vec2 tcs_uv[];\n\
        in vec3 tcs_pos[];\n\
        in vec3 tcs_normal[];\n\
        out vec2 tes_uv;\n\
        out vec3 tes_pos;\n\
        out vec3 tes_normal;\n\
        void main() {\n\
            //get x and y from the tessellation coord and lerp them using the glsl's mix function\n\
            float x = gl_TessCoord.x;\n\
            float y = gl_TessCoord.y;\n\
            //for position\n\
            vec3 quad_bot_pos = mix(tcs_pos[0], tcs_pos[1], x);\n\
            vec3 quad_top_pos = mix(tcs_pos[3], tcs_pos[2], x);\n\
            tes_pos = mix(quad_bot_pos, quad_top_pos, y);\n\
            //for uv\n\
            vec2 quad_bot_uv = mix(tcs_uv[0], tcs_uv[1], x);\n\
            vec2 quad_top_uv = mix(tcs_uv[3], tcs_uv[2], x);\n\
            tes_uv = mix(quad_bot_uv, quad_top_uv, y);\n\
            //for normal\n\
            vec3 quad_bot_normal = mix(tcs_normal[0], tcs_normal[1], x);\n\
            vec3 quad_top_normal = mix(tcs_normal[3], tcs_normal[2], x);\n\
            tes_normal = mix(quad_bot_normal, quad_top_normal, y);\n\
        }\n";
        std::string GeoShaderCode = "\
        #version 400 core\n\
        //expected triangle in\n\
        layout(triangles) in;\n\
        //triangle_strip out\n\
        layout(triangle_strip, max_vertices = 3) out;\n\
        uniform float time;\n\
        uniform mat4 mvp;\n\
        in vec2 tes_uv[];\n\
        in vec3 tes_pos[];\n\
        out vec2 gs_uv;\n\
        out vec3 gs_normal;\n\
        out vec3 gs_vertpos;\n\
        uniform sampler2D dispMap;\n\
        //Gerstner wave.Pretty much just follow the equations provided for x,y,z\n\
        vec3 Gerstner(vec3 worldpos, float w, float A, float phi, float Q, vec2 D, int N) {\n\
            vec3 pos = vec3(0.0);\n\
            float dot_product = dot(w * D, worldpos.xz);\n\
            pos.x = Q * A * D.x * cos(dot_product + phi * time);\n\
            pos.y = A * sin(dot_product + phi * time);\n\
            pos.z = Q * A * D.y * cos(dot_product + phi * time);\n\
            return pos;\n\
        }\n\
        void main() {\n\
            //world space for wave position\n\
            vec3 positions[3];\n\
            //apply Gerstner wave through each vertex in the triangle\n\
            for (int i = 0; i < tes_pos.length(); i++) {\n\
                vec3 position = tes_pos[i];\n\
                float displacement = texture(dispMap, tes_uv[i]).r;\n\
                position.y += displacement;\n\
                // Combine multiple Gerstner waves provided in the assingment file\n\
                position += Gerstner(tes_pos[i], 4.0, 0.08, 1.1, 0.75, vec2(0.3, 0.6), 1);\n\
                position += Gerstner(tes_pos[i], 2, 0.05, 1.1, 0.75, vec2(0.2, 0.866), 1);\n\
                position += Gerstner(tes_pos[i], 0.6, 0.2, 0.4, 0.1, vec2(0.3, 0.7), 1); \n\
                position += Gerstner(tes_pos[i], 0.9, 0.15, 0.4, 0.1, vec2(0.8, 0.1), 1);\n\
                positions[i] = position;\n\
            }\n\
            //normal calculation to update in real-time\n\
            vec3 normal=normalize(cross( positions[1]-positions[0], positions[2]-positions[0]));\n\
            for (int j = 0; j < positions.length(); j++) {\n\
                //Apply MVP to transform the generated position to clip space\n\
                gl_Position = mvp * vec4(positions[j], 1.0);\n\
                gs_uv = tes_uv[j];\n\
                gs_normal = normal; \n\
                gs_vertpos = positions[j];\n\
                EmitVertex();\n\
            }\n\
            EndPrimitive();\n\
        }\n";

        std::string FragShaderCode = "\
        #version 400 core\n\
        out vec4 fragCol;\n\
        in vec2 gs_uv;\n\
        in vec3 gs_normal;\n\
        in vec3 gs_vertpos;\n\
        uniform sampler2D tex;\n\
        uniform vec3 eyeDir;\n\
        uniform vec3 lightDir;\n\
        void main() {\n\
            vec3 normLightDir = normalize(-lightDir);\n\
            vec3 normal = normalize(gs_normal);\n\
            //use the water texturee as ambient. Dimmed since it is hard to show specular\n\
            vec3 ambient = vec3(0.75,0.75,0.75)*texture(tex, gs_uv).rgb;\n\
            //diffuse light calculation\n\
            vec3 diffuseColor = texture(tex, gs_uv).rgb;\n\
            float diffuseFactor = max(dot(normal, normLightDir), 0);\n\
            vec3 diffuse = diffuseColor * diffuseFactor;\n\
            //specular light calculation\n\
            vec3 specularColor = vec3(1, 1, 1);\n\
            vec3 reflection = reflect(-normLightDir, normal);\n\
            vec3 eyedir = normalize(eyeDir - gs_vertpos);\n\
            float specularCoefficient = pow(max(dot(eyedir, reflection), 0), 16);\n\
            vec3 specular = specularColor * specularCoefficient;\n\
            fragCol = vec4(ambient + diffuse + specular, 1.0);\n\
        }\n";
        const char* c_vscode = VertexShaderCode.c_str();
        const char* c_tcscode = TCShaderCode.c_str();
        const char* c_tescode = TEShaderCode.c_str();
        const char* c_geocode = GeoShaderCode.c_str();
        const char* c_fscode = FragShaderCode.c_str();
        //Generate shader ID
        unsigned int vshader = glCreateShader(GL_VERTEX_SHADER);
        unsigned int tcshader = glCreateShader(GL_TESS_CONTROL_SHADER);
        unsigned int tesshader = glCreateShader(GL_TESS_EVALUATION_SHADER);
        unsigned int geoshader = glCreateShader(GL_GEOMETRY_SHADER);
        unsigned int fshader = glCreateShader(GL_FRAGMENT_SHADER);
        //Point to source code
        glShaderSource(vshader, 1, &c_vscode, NULL);
        glShaderSource(fshader, 1, &c_fscode, NULL);
        glShaderSource(tcshader, 1, &c_tcscode, NULL);
        glShaderSource(tesshader, 1, &c_tescode, NULL);
        glShaderSource(geoshader, 1, &c_geocode, NULL);
        //Compile Shader
        glCompileShader(vshader);
        glCompileShader(fshader);
        glCompileShader(geoshader);
        glCompileShader(tcshader);
        glCompileShader(tesshader);

        //Create program and attach shaders
        unsigned int program = glCreateProgram();
        glAttachShader(program, vshader);
        glAttachShader(program, fshader);
        glAttachShader(program, tcshader);
        glAttachShader(program, tesshader);
        glAttachShader(program, geoshader);
        //Validate and link program
        glValidateProgram(program);
        glLinkProgram(program);
        //Delete shader after linking to program.
        glDeleteShader(vshader);
        glDeleteShader(fshader);
        glDeleteShader(tesshader);
        glDeleteShader(tcshader);
        glDeleteShader(geoshader);


        return program;
    }
};

#endif