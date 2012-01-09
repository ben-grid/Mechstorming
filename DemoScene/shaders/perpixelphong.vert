//  Per-pixel Phong lighting
//  Vertex shader

varying vec3 View;
varying vec3 Light;
varying vec3 Normal;
varying vec4 Ambient;
varying vec3 position;
varying vec3 worldNormal;
varying vec3 eyeNormal;

uniform sampler2D heightMap;
uniform sampler2D baseMap1;
uniform sampler2D baseMap2;
uniform int mode;

// water stuff
const float pi = 3.1415925358;
uniform float time;
uniform int numWaves;
uniform float amplitude[8];
uniform float wavelength[8];
uniform float speed[8];
uniform vec2 direction[8];

float makeWave(int i, float x, float y){
    float freq = 2*pi/wavelength[i];
    float phase = speed[i]*freq;
    float theta = dot(direction[i], vec2(x,y));
    return amplitude[i]*sin(theta*freq+ time*phase);
}

float waveHeight(float x, float y){
    float height = 0.0;
    for(int i=0; i< numWaves; ++i){
	height+= makeWave(i,x,y);
    }
    return height;
}

float dWavedx(int i, float x, float y){
    float frequency = 2*pi/wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], vec2(x, y));
    float A = amplitude[i] * direction[i].x * frequency;
    return A * cos(theta * frequency + time * phase);
}

float dWavedy(int i, float x, float y){
    float frequency = 2*pi/wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], vec2(x, y));
    float A = amplitude[i] * direction[i].y * frequency;
    return A * cos(theta * frequency + time * phase);
}

vec3 waveNormal(float x, float y){
    float dx = 0.0;
    float dy = 0.0;
    for (int i = 0; i < numWaves; ++i) {
        dx += dWavedx(i, x, y);
        dy += dWavedy(i, x, y);
    }
    vec3 n = vec3(-dx, -dy, 1.0);
    return normalize(n);
}

void main()
{
   //
   //  Lighting values needed by fragment shader
   //
   //  Vertex location in modelview coordinates
   vec3 P = vec3(gl_ModelViewMatrix * gl_Vertex);
   //  Light position
   Light  = vec3(gl_LightSource[0].position) - P;
   //  Normal
   Normal = gl_NormalMatrix * gl_Normal;
   //  Eye position
   View  = -P;
   //  Ambient color
   Ambient = gl_FrontMaterial.emission + gl_FrontLightProduct[0].ambient + gl_LightModel.ambient*gl_FrontMaterial.ambient;


    switch(mode){
	case 0:
   	//  Texture coordinate for fragment shader
	vec4 pos = gl_Vertex;
   	gl_TexCoord[0] = gl_MultiTexCoord0;
   	gl_TexCoord[1].st = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t)*8;
   	pos.z += 200*texture2D(heightMap, gl_TexCoord[0].st).a;
	//  Set vertex position
	gl_Position = gl_ModelViewProjectionMatrix * pos; //ftransform();
	break;

	case 1:
	gl_TexCoord[3] = gl_MultiTexCoord3;
   	//  Set vertex position
	gl_Position = ftransform();
	break;

	case 2:
	vec4 vPos = gl_Vertex;
	vPos.z += waveHeight(vPos.x, vPos.y);
	position = vPos.xyz/ vPos.w;
	worldNormal = waveNormal(vPos.x, vPos.y);
	Normal = gl_NormalMatrix * worldNormal;
	gl_Position = gl_ModelViewProjectionMatrix * vPos;
	gl_FrontColor = gl_Color;
	break;
   }
}
