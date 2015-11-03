#include "sphere.h"
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <iostream>
/**********************************************************************
 * This function intersects a ray with a given sphere 'sph'. You should
 * use the parametric representation of a line and do the intersection.
 * The function should return the parameter value for the intersection, 
 * which will be compared with others to determine which intersection
 * is closest. The value -1.0 is returned if there is no intersection
 *
 * If there is an intersection, the point of intersection should be
 * stored in the "hit" variable
 **********************************************************************/
float intersect_sphere(Point o, Vector u, Spheres *sph, Point *hit, Point *hit2) 
{
	Vector oc = get_vec(o, sph->center);
  	float  d = vec_dot(u, oc)/vec_len(u);
  	float dis_square = pow(vec_len(oc),2) - d*d;
  	// if no intersection
  	if (dis_square > pow(sph->radius, 2))
    	return -1.0;
  	// if there is intersection
  	float dis_to_hit = d - sqrt( pow(sph->radius, 2) - dis_square );
    float dis_to_hit2 = d + sqrt( pow(sph->radius, 2) - dis_square );
  	if (dis_to_hit < 0)
  		return -1.0;
  	Vector v = {u.x, u.y, u.z};
  	normalize(&v);
  	*hit = get_point(o,  vec_scale(v, dis_to_hit) );
    *hit2 = get_point(o,  vec_scale(v, dis_to_hit2) );
		return dis_to_hit;
}

/*********************************************************************
 * This function returns a pointer to the sphere object that the
 * ray intersects first; NULL if no intersection. You should decide
 * which arguments to use for the function. For exmaple, note that you
 * should return the point of intersection to the calling function.
 **********************************************************************/
Spheres *intersect_scene(Point o, Vector u, Spheres *slist, Point *hit, Point *hit2) {


	float shortest_dist = 100000;
	float dist;
	Spheres *sphere = slist;
	Spheres *closest_sphere = NULL;

	while(sphere != NULL)
	{
		dist = intersect_sphere(o, u, sphere, hit, hit2);
		// if no intersection
		if (dist == -1)
		{
			sphere = sphere->next;
			continue;
		}
		// if intersected
		if( dist < shortest_dist )
		{
			shortest_dist = dist; 
			closest_sphere = sphere;
		 }//intersects a sphere
		
		sphere = sphere->next;
		
	}
	return closest_sphere;
}


/*****************************************************
 * This function adds a sphere into the sphere list
 *
 * You need not change this.
 *****************************************************/
Spheres *add_sphere(Spheres *slist, Point ctr, float rad, float amb[],float dif[], 
	float spe[], float shine, float refl,  int sindex, float refr, float refr_coef) 
{
  Spheres *new_sphere;

  new_sphere = (Spheres *)malloc(sizeof(Spheres));
  new_sphere->index = sindex;
  new_sphere->center = ctr;
  new_sphere->radius = rad;
  (new_sphere->mat_ambient)[0] = amb[0];
  (new_sphere->mat_ambient)[1] = amb[1];
  (new_sphere->mat_ambient)[2] = amb[2];
  (new_sphere->mat_diffuse)[0] = dif[0];
  (new_sphere->mat_diffuse)[1] = dif[1];
  (new_sphere->mat_diffuse)[2] = dif[2];
  (new_sphere->mat_specular)[0] = spe[0];
  (new_sphere->mat_specular)[1] = spe[1];
  (new_sphere->mat_specular)[2] = spe[2];
  new_sphere->mat_shineness = shine;
  new_sphere->reflectance = refl;
  new_sphere->refraction = refr;
  new_sphere->kt = refr_coef;
  new_sphere->next = NULL;

  if (slist == NULL) { // first object
    slist = new_sphere;
  } else { // insert at the beginning
    new_sphere->next = slist;
    slist = new_sphere;
  }

  return slist;
}


/******************************************
 * computes a sphere normal - done for you
 ******************************************/
Vector sphere_normal(Point q, Spheres *sph) {
  Vector rc;

  rc = get_vec(sph->center, q);
  normalize(&rc);
  return rc;
}