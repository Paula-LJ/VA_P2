varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;

varying vec2 v_uv;
uniform samplerCube texture;

//Variables Phong
uniform vec3 lightpos;
uniform vec3 Ia;
uniform vec3 Is;
uniform vec3 Id;
uniform float alpha;
uniform vec3 u_camera_pos; 

void main()
{
	vec3 I = normalize(v_world_position - u_camera_pos);
	vec3 Ref = reflect(I, normalize(v_normal));
	gl_FragColor = textureCube(texture,Ref); 
}