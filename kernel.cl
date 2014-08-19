kernel void simulation(global float4* pos, global float4* vel,float timeScale)
{
	const float g = 6.67384e-11;
    //get workunit in the array
    unsigned int i = get_global_id(0);

	float4 d = -pos[i];

	float r = length(d);

	float force = (g * 10E04) / (r*r);
	vel[i] += d * force * timeScale / r;

	pos[i] += vel[i] * timeScale;
}