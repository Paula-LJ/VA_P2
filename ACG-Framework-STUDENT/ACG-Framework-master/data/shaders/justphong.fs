varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;

varying vec2 v_uv;

//Variables Phong
uniform vec3 lightpos;
uniform vec3 Ia;
uniform vec3 Is;
uniform vec3 Id;
uniform float alpha;
uniform vec3 u_camera_pos; 

void main()
{

	vec3 color = (1.0,1.0,1.0);
	vec3 normal= normalize(v_normal); 
	vec2 uv = v_uv;

	//Computations for PHONG.
	vec3 L = lightpos - v_world_position; 
	L = normalize(L);
		
	vec3 R = reflect(L, normal);
	R = normalize(R);
	
	vec3 V = u_camera_pos-v_world_position;
	V = normalize(V);
	
	float diffuse = clamp(dot(L,normal),0.0,1.0);
	float specular = clamp(dot(-R, V),0.0,1.0);
	
	vec3 ambient_color = Ia;
	vec3 diffuse_color = diffuse*Id;
	vec3 specular_color = pow(specular, alpha)*Is;

	color = ambient_color + diffuse_color + specular_color;

	gl_FragColor = vec4(color,1.0);
}