//  Per-pixel Phong lighting
//  Fragment shader

varying vec3 View;
varying vec3 Light;
varying vec3 Normal;
varying vec4 Ambient;

varying vec3 position; 
varying vec3 worldNormal;

uniform sampler2D heightMap;
uniform sampler2D baseMap1;
uniform sampler2D baseMap2;
uniform sampler2D skybox;
uniform samplerCube envMap;
uniform int mode;

vec4 phong()
{
   //  N is the object normal
   vec3 N = normalize(Normal);
   //  L is the light vector
   vec3 L = normalize(Light);

   //  Emission and ambient color
   vec4 color = Ambient;

   //  Diffuse light is cosine of light and normal vectors
   float Id = dot(L,N);
   if (Id>0.0)
   {
      //  Add diffuse
      color += Id*gl_FrontLightProduct[0].diffuse;
      //  R is the reflected light vector R = 2(L.N)N - L
      vec3 R = reflect(-L,N);
      //  V is the view vector (eye vector)
      vec3 V = normalize(View);
      //  Specular is cosine of reflected and view vectors
      float Is = dot(R,V);
      if (Is>0.0) color += pow(Is,gl_FrontMaterial.shininess)*gl_FrontLightProduct[0].specular;
   }

   //  Return sum of color components
   return color;
}

void main()
{
    switch(mode){
	case 0:
	    vec4 grass = texture2D(baseMap1, gl_TexCoord[1].xy);
	    vec4 dirt = texture2D(baseMap2, gl_TexCoord[1].xy);
	    vec4 m = mix(dirt, grass, clamp(vec4(texture2D(heightMap, gl_TexCoord[0].xy).a)+.3, vec4(0,0,0,1), vec4(1,1,1,1)));
	    gl_FragColor = phong() * m;
	    break;
	case 1:
	    gl_FragColor = texture2D(skybox, gl_TexCoord[3].xy);
	    break;

	case 2:
	    vec3 eye = normalize(View - position);
	    vec3 r = reflect( eye, worldNormal);
	    vec4 color = textureCube(envMap, r);
	    color.a = .2;
	    gl_FragColor =  gl_Color * phong();//color;
	    break;
    }
}
