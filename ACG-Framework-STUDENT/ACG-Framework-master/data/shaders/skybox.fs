
varying vec3 v_world_position;

uniform samplerCube u_texture;
uniform vec3 u_camera_pos;

void main()
{
	vec3 direction = normalize(v_world_position-u_camera_pos);
	gl_FragColor =  textureCube( u_texture, direction );
}