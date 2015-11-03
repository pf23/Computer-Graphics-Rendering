#include <stdio.h>
#include <time.h>
#include <GL/glut.h>
#include <math.h>
#include "global.h"
#include "sphere.h"
#include <iostream>
//
// Global variables
//
extern int win_width;
extern int win_height;

extern GLfloat frame[WIN_HEIGHT][WIN_WIDTH][3];  

extern float image_width;
extern float image_height;

extern Point eye_pos;
extern float image_plane;
extern RGB_float background_clr;

extern Spheres *scene;

// light 1 position and color
extern Point light1;
extern float light1_ambient[3];
extern float light1_diffuse[3];
extern float light1_specular[3];

// global ambient term
extern float global_ambient[3];
extern float global_reflect[3];

// light decay parameters
extern float decay_a;
extern float decay_b;
extern float decay_c;

// boolean flags for features
extern bool shadow_on;
extern bool reflection_on;
extern bool chessboard_on;
extern bool refraction_on;
extern bool stochastic_ray_on;
extern bool supersampling_on;

// maximal reflection and refraction steps
extern int step_max;

/**********************************************
/ 	 set chess board parameters here
**********************************************/
Vector chessboard_norm= { 0, 1, 0 };
Point board_center = { 0, -3, -7};
float board_width = 8;
float board_ambi[] = {0.3, 0.3, 0.3};
float board_reflec = 0.4;
float board_refrac = 1.53;

//therefore the range of the plane is
float x_left = board_center.x - board_width/2;
float x_right = board_center.x + board_width/2;
float z_front = board_center.z + board_width/2;
float z_rear = board_center.z - board_width/2;

//-----------------------------------------------
// get the ambient color of an object given its ambient coef
RGB_float ambient_color(float *ambi)
{
	RGB_float ambient = {0,0,0};

	//global 
	ambient.r += global_ambient[0] * global_reflect[0];
	ambient.g += global_ambient[1] * global_reflect[1];
	ambient.b += global_ambient[2] * global_reflect[2];

	//local 
	ambient.r += light1_ambient[0] * ambi[0];
	ambient.g += light1_ambient[1] * ambi[1];
	ambient.b += light1_ambient[2] * ambi[2];

	return ambient;
}

// check whether blocked by spheres
bool ShadowFeeler (Point eye, Vector u, Spheres *slist, Spheres *cur)
{
	
	Point *hit = new Point;
	Point *hit2 = new Point;

	while(slist)
	{
		if (slist == cur)
		{
			slist = slist->next;
			continue;
		}
		if (intersect_sphere(eye, u, slist, hit, hit2) != -1)
			return true;
		slist = slist->next;
	}

	return false;
}

// check whether a ray intersects with the board
bool intersect_plane(Point p, Vector u, Vector chessboard_norm, Point p_on_board , Point *hit)
{
	float numerator = vec_dot(chessboard_norm, get_vec(p_on_board, p));
	float denominator = vec_dot(chessboard_norm, u);
	float t = -numerator/denominator;

	// no intersection
	if( denominator == 0 || t < 0) 
		return false;
	// get hit point
	*hit = get_point(p, vec_scale(u, t));
	// hit point exceeds the border
	if ( hit->x > x_right || hit->x < x_left || hit->z > z_front || hit->z < z_rear)
		return false;
	
	return true;
}

// color the chess board by pixels
RGB_float plane_color(Point p)
{
	RGB_float color;
	int x,z;
	if( p.x < 0) x = int(p.x) + 1;
	else x = int(p.x);
	if( p.z < 0) z = int(p.z) + 1;
	else z = int(p.z);
	
	int index = ( x%2 == 0)*10 + ( z%2 == 0);
	switch(index)
	{
		case 0:case 11: color = {0.0, 0.0, 0.0};break;
		case 1:case 10: color = {1.0, 1.0, 1.0};break;
	}
	//color = clr_add(color, income_ray);
	return color;
}

// calculate the refracive vector
Vector refrac_vec(bool inside, float in_fract_index, float out_fract_index, Vector surf_norm, Vector v)
{
	Vector refract;

	float n = in_fract_index / out_fract_index;
	if(inside)
		n = out_fract_index / in_fract_index;

	float temp1 = vec_dot(surf_norm, vec_scale(v,-1));
	float temp2 = sqrt(1.0-pow(n,2)*(1-pow(temp1,2)));
	
	Vector vec_temp1;
	vec_temp1 = vec_scale(surf_norm, n*temp1 - temp2);

	refract = vec_plus(vec_temp1, vec_scale(v, n));
	// from sphere to air, possibly total refelction
	if (!inside)
		return refract;
	// calculate the critical angle
	float crit_angle = asin( in_fract_index / out_fract_index )/3.1415926 * 180;

	// calculate the angel of normal and in_refraction ray, if exceeding critcal value
	// there is total reflection
	normalize(&surf_norm);
	normalize(&v);
	float cosine = vec_dot(vec_scale(surf_norm, -1), v);
	float angel = acos(cosine)/3.1415926 * 180;
	if ( angel < crit_angle )
	{
		//printf("non-total\n");
		return refract;
	}
	else
	{
		// there will be no outgoing refraction ray
		//printf ("total\n");
		return { 0, 0, 0 };
	}
}
/////////////////////////////////////////////////////////////////////

/*********************************************************************
 * Phong illumination - you need to implement this!
 *********************************************************************/
RGB_float phong(Point q, Vector viewer, Vector surf_norm, Spheres *sph) {
//
// do your thing here
//
	RGB_float ambient = ambient_color(sph->mat_ambient);
	RGB_float color = { 0, 0, 0 };

	// get light vec
	Vector light_vec = get_vec(q, light1);
	float d = vec_len(get_vec(q, light1));
	normalize(&light_vec);

	// get reflectance vec
	float dot_prod = vec_dot(surf_norm, vec_scale(light_vec,-1));
	Vector reflect_vec = vec_plus(vec_scale(surf_norm, 2*dot_prod), light_vec);
	//normalize(&reflect_vec);

	// get attenuation parameter
	float attenuation = 1/(decay_a + decay_b*d + decay_c*pow(d,2));
	//printf("%d\n", attenuation);
	if (attenuation > 1)
		attenuation = 1;
	
	//diffuse
	float dot_prod1 = vec_dot(surf_norm, light_vec);
	if(dot_prod1 < 0)
		dot_prod1 = 0;
	color.r += attenuation * light1_diffuse[0] * sph->mat_diffuse[0] * dot_prod1;
	color.g += attenuation * light1_diffuse[1] * sph->mat_diffuse[1] * dot_prod1; 
	color.b += attenuation * light1_diffuse[2] * sph->mat_diffuse[2] * dot_prod1;

	//specular
	float dot_prod2 = vec_dot(reflect_vec, vec_scale(viewer,-1) );
	if (dot_prod2 < 0)
		dot_prod2 = 0;
	color.r += attenuation * light1_specular[0] * sph->mat_specular[0] * pow(dot_prod2, sph->mat_shineness);
	color.g += attenuation * light1_specular[1] * sph->mat_specular[1] * pow(dot_prod2, sph->mat_shineness);
	color.b += attenuation * light1_specular[2] * sph->mat_specular[2] * pow(dot_prod2, sph->mat_shineness);

	// add shadow
	if( ShadowFeeler(q, light_vec, scene, sph) && shadow_on)
		return ambient;
	else
	{
		color = clr_add(color, ambient);
		return color;
	}
}


/************************************************************************
 * This is the recursive ray tracer - you need to implement this!
 * You should decide what arguments to use.
 ************************************************************************/
RGB_float recursive_ray_trace(bool inside, Vector u, Point p, int step) 
{
//
// do your thing here
//
	RGB_float color = background_clr;
	RGB_float reflected_color = { 0, 0, 0 };
	RGB_float refracted_color = { 0, 0, 0 };
	
	Spheres *closest_sph;
	Point *hit_sph = new Point;
	Point *hit_sph2 = new Point;
	closest_sph = intersect_scene(p, u, scene, hit_sph, hit_sph2);

	// if it hits plane
	Point *hit_plane = new Point;
	if(chessboard_on && intersect_plane(p, u, chessboard_norm, board_center, hit_plane))
	{
		color = plane_color(*hit_plane);
		// if there is shadow
		if(shadow_on && ShadowFeeler(*hit_plane, get_vec(*hit_plane, light1), scene, NULL) )
			//color = ambient_color(board_ambi);
			color = clr_add (clr_scale(color, 0.3), ambient_color(board_ambi) );
		else
			color = clr_add (color, ambient_color(board_ambi) );
	}

	if(closest_sph == NULL)
		return color;

	// if the ray is inside the sphere
	if(inside)
		*hit_sph = *hit_sph2;

	// if it hits a sphere first, reset the color
	Vector surf_norm = sphere_normal(*hit_sph, closest_sph);
	Vector ray = get_vec(*hit_sph, p);
	normalize(&ray);
	normalize(&u);

	color = phong(*hit_sph, u, surf_norm, closest_sph);

	if(step < step_max)
		step++;
	else
		return color;

	if(reflection_on)
	{
		Vector reflect_vec = vec_minus(vec_scale(surf_norm, vec_dot(surf_norm, ray)*2), ray);
		reflected_color = recursive_ray_trace(inside, reflect_vec, *hit_sph, step);
		reflected_color = clr_scale(reflected_color, closest_sph->reflectance);
		color = clr_add(color, reflected_color);

		// diffuse reflection is applied only when reflection is applied
		if(stochastic_ray_on)
		{
			RGB_float new_color = {0,0,0};
			float flag[3] = { 1, 1, 1 };
			if (surf_norm.x < 0)flag[0] = -1;
			if (surf_norm.y < 0)flag[1] = -1;
			if (surf_norm.z < 0)flag[2] = -1;
			for (int i=0; i < DIFFUSE_RAYS; i++)
			{
				Vector new_norm;
				// since these three spheres are quite smooth, 
				// the diffuse norm can't differ from the surf norm too much
				srand((int)time(0));
				new_norm.x = surf_norm.x + .05*flag[0] * rand()/ float (RAND_MAX);
				new_norm.y = surf_norm.y + .05*flag[0] * rand()/ float (RAND_MAX);
				new_norm.z = surf_norm.z + .05*flag[0] * rand()/ float (RAND_MAX);
				normalize(&new_norm);
				reflect_vec = vec_minus(vec_scale(new_norm, vec_dot(new_norm, ray)*2), ray);
				reflected_color = recursive_ray_trace(inside, reflect_vec, *hit_sph, step);
				reflected_color = clr_scale(reflected_color, closest_sph->reflectance);
				new_color = clr_add(new_color, reflected_color);
			}
			// set a small contribution of the diffuse lights
			new_color  = clr_scale(new_color, .005); 
			color = clr_add(color, new_color);
		}

	}

	if(refraction_on)
	{
		Vector refracted_ray = refrac_vec(inside, 1.0029, closest_sph->refraction, surf_norm, ray);
		// if there is no total reflection
		if ( vec_len( refracted_ray ) != 0 )
		{
			inside = !inside;
			// move a little bit back along the ray vector to avoid being in the sphere
			*hit_sph = get_point( *hit_sph, vec_scale( refracted_ray, -1) );
			refracted_color = recursive_ray_trace(inside, refracted_ray, *hit_sph, step);
			refracted_color = clr_scale(refracted_color, closest_sph->kt);
			color = clr_add(color, refracted_color);
		}
		
	}
	return color;
}


//
// for super sampling to reduce aliasing
//
 RGB_float supersampling(Point cur_pixel_pos, float x_grid_size, 
 		float y_grid_size, RGB_float color)
 {
	RGB_float ret_color = color;
	Vector ray;
	Point pixel = cur_pixel_pos;

	for(float dx = -x_grid_size/2; dx <= x_grid_size/2; dx += x_grid_size)
	{
		pixel.x = cur_pixel_pos.x + dx;
		for(float dy = -y_grid_size/2; dy <= y_grid_size/2; dy += y_grid_size)
		{
			pixel.y = cur_pixel_pos.y + dy;
			ray = get_vec(eye_pos, pixel);
			ret_color = clr_add(ret_color, recursive_ray_trace(false, ray, eye_pos, 0));
		}
	}
	clr_scale(ret_color, 0.2);
	return ret_color;
 }

/*********************************************************************
 * This function traverses all the pixels and cast rays. It calls the
 * recursive ray tracer and assign return color to frame
 *
 * You should not need to change it except for the call to the recursive
 * ray tracer. Feel free to change other parts of the function however,
 * if you must.
 *********************************************************************/
void ray_trace() {
	int i, j;
	float x_grid_size = image_width / float(win_width);
	float y_grid_size = image_height / float(win_height);
	float x_start = -0.5 * image_width;
	float y_start = -0.5 * image_height;
	RGB_float ret_color;
	Point cur_pixel_pos;
	Vector ray;

	// ray is cast through center of pixel
	cur_pixel_pos.x = x_start + 0.5 * x_grid_size;
	cur_pixel_pos.y = y_start + 0.5 * y_grid_size;
	cur_pixel_pos.z = image_plane;

	for (i=0; i<win_height; i++) {
		for (j=0; j<win_width; j++) {

			ray = get_vec(eye_pos, cur_pixel_pos);
			//
      		// You need to change this!!!
      		//
			ret_color = recursive_ray_trace(false, ray, eye_pos, 0);
			if(supersampling_on)
			{
				ret_color = supersampling(cur_pixel_pos, x_grid_size, 
					y_grid_size, ret_color);
			}
			frame[i][j][0] = GLfloat(ret_color.r);
			frame[i][j][1] = GLfloat(ret_color.g);
			frame[i][j][2] = GLfloat(ret_color.b);

			cur_pixel_pos.x += x_grid_size;
		}
		cur_pixel_pos.y += y_grid_size;
		cur_pixel_pos.x = x_start;
	}
}