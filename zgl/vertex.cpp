#include <string.h>
#include <GL/zgl.h>



void glopNormal(GLContext * c, GLParam * p)
{
    c->current_normal.x = p[1].f;
    c->current_normal.y = p[2].f;
    c->current_normal.z = p[3].f;
    c->current_normal.w = (float)(0);
}

void glopTexCoord(GLContext * c, GLParam * p)
{
    c->current_tex_coord.x = p[1].f;
    c->current_tex_coord.y = p[2].f;
    c->current_tex_coord.z = p[3].f;
    c->current_tex_coord.w = p[4].f;
}

void glopEdgeFlag(GLContext * c, GLParam * p)
{
    c->current_edge_flag = p[1].i;
}

void glopColor(GLContext * c, GLParam * p)
{
    c->current_color.x = p[1].f;
    c->current_color.y = p[2].f;
    c->current_color.z = p[3].f;
    c->current_color.w = p[4].f;
    c->longcurrent_color[0] = p[5].ui;
    c->longcurrent_color[1] = p[6].ui;
    c->longcurrent_color[2] = p[7].ui;
    
    if (c->color_material_enabled) 
	{
		GLParam q[7];
		q[0].op = OP_Material;
		q[1].i = c->current_color_material_mode;
		q[2].i = c->current_color_material_type;
		q[3].f = p[1].f;
		q[4].f = p[2].f;
		q[5].f = p[3].f;
		q[6].f = p[4].f;
		glopMaterial(c, q);
    }
}


void gl_eval_viewport(GLContext * c)
{
    GLViewport *v;
    GLfloat zsize = (float)(1 << (ZB_Z_BITS + ZB_POINT_Z_FRAC_BITS));

    GLfloat tmp05=(0.5);
    GLfloat tmp20=(float)(2);
    GLfloat tmpx=(float)(c->viewport.xsize);
    GLfloat tmpy=(float)(c->viewport.ysize);

	v = &c->viewport;

    v->trans.x = (((tmpx- tmp05)/ tmp20)+ (float)(v->xmin));
    v->trans.y = (((tmpy- tmp05)/ tmp20)+ (float)(v->ymin));
    v->trans.z = (((zsize- tmp05)/ tmp20)+ ((float)(1 << ZB_POINT_Z_FRAC_BITS)/ tmp20));

    v->scale.x = ((tmpx- tmp05)/ tmp20);
    v->scale.y = -(((tmpy- tmp05)/ tmp20));
    v->scale.z = -(((zsize- tmp05)/ tmp20));
}

void glopBegin(GLContext * c, GLParam * p)
{
    int type;
    glm::mat4 tmp;

    assert(c->in_begin == 0);

    type = p[1].i;
    c->begin_type = type;
    c->in_begin = 1;
    c->vertex_n = 0;
    c->vertex_cnt = 0;

    if (c->matrix_model_projection_updated) 
	{
		if (c->lighting_enabled) 
		{
			/* precompute inverse modelview */
			tmp = glm::inverse(*c->matrix_stack_ptr[0]);
			c->matrix_model_view_inv = glm::transpose(tmp);
		} 
		else 
		{
			float *m = glm::value_ptr(c->matrix_model_projection);
			/* precompute projection matrix */
			c->matrix_model_projection = c->matrix_stack_ptr[0][0] * c->matrix_stack_ptr[1][0];
			  
			/* test to accelerate computation */
			c->matrix_model_projection_no_w_transform = 0;
		
			if ((float)(m[12]) == (float)((float)(0)) && 
				(float)(m[13]) == (float)((float)(0)) && 
				(float)(m[14]) == (float)((float)(0)))
				c->matrix_model_projection_no_w_transform = 1;
				//
				// 0 1 2 3
				// 4 5 6 7
				// 8 9 10 11
				// 12 13 14 15
			
			/*
			if (c->matrix_model_projection[3][0] == 0 &&
				c->matrix_model_projection[3][1] == 0 &&
				c->matrix_model_projection[3][2] == 0)
				c->matrix_model_projection_no_w_transform = 1;
				*/
		}

		/* test if the texture matrix is not Identity */
		c->apply_texture_matrix = (c->matrix_stack_ptr[2][0] != glm::mat4());

		c->matrix_model_projection_updated = 0;
    }
    /*  viewport */
    if (c->viewport.updated) 
	{
		gl_eval_viewport(c);
		c->viewport.updated = 0;
    }
    /* triangle drawing functions */
    if (c->render_mode == GL_SELECT) 
	{
		c->draw_triangle_front = gl_draw_triangle_select;
		c->draw_triangle_back = gl_draw_triangle_select;
    } 
	else 
	{
		switch (c->polygon_mode_front) 
		{
		case GL_POINT:
			c->draw_triangle_front = gl_draw_triangle_point;
			break;
		case GL_LINE:
			c->draw_triangle_front = gl_draw_triangle_line;
			break;
		default:
			c->draw_triangle_front = gl_draw_triangle_fill;
			break;
		}

		switch (c->polygon_mode_back) 
		{
		case GL_POINT:
			c->draw_triangle_back = gl_draw_triangle_point;
			break;
		case GL_LINE:
			c->draw_triangle_back = gl_draw_triangle_line;
			break;
		default:
			c->draw_triangle_back = gl_draw_triangle_fill;
			break;
		}
    }
}


/* coords, tranformation , clip code and projection */
/* TODO : handle all cases */
static inline void gl_vertex_transform(GLContext * c, GLVertex * v)
{
    glm::vec4 *n;
	float *m;

    if (c->lighting_enabled) 
	{
		/* eye coordinates needed for lighting */

		m = glm::value_ptr(*c->matrix_stack_ptr[0]);
		v->ec.x = ((((v->coord.x* m[0])+(v->coord.y* m[1]))+(v->coord.z* m[2]))+m[3]);
		v->ec.y = ((((v->coord.x* m[4])+(v->coord.y* m[5]))+(v->coord.z* m[6]))+m[7]);
		v->ec.z = ((((v->coord.x* m[8])+(v->coord.y* m[9]))+(v->coord.z* m[10]))+m[11]);
		v->ec.w = ((((v->coord.x* m[12])+(v->coord.y* m[13]))+(v->coord.z* m[14]))+m[15]);

		/* projection coordinates */
		m = glm::value_ptr(*c->matrix_stack_ptr[1]);
		v->pc.x = ((((v->ec.x* m[0])+(v->ec.y* m[1]))+(v->ec.z* m[2]))+(v->ec.w* m[3]));
		v->pc.y = ((((v->ec.x* m[4])+(v->ec.y* m[5]))+(v->ec.z* m[6]))+(v->ec.w* m[7]));
		v->pc.z = ((((v->ec.x* m[8])+(v->ec.y* m[9]))+(v->ec.z* m[10]))+(v->ec.w* m[11]));
		v->pc.w = ((((v->ec.x* m[12])+(v->ec.y* m[13]))+(v->ec.z* m[14]))+(v->ec.w* m[15]));

		m = glm::value_ptr(c->matrix_model_view_inv);

		n = &c->current_normal;

		v->normal.x = (((n->x* m[0])+(n->y* m[1]))+(n->z* m[2]));
		v->normal.y = (((n->x* m[4])+(n->y* m[5]))+(n->z* m[6]));
		v->normal.z = (((n->x* m[8])+(n->y* m[9]))+(n->z* m[10]));

		if (c->normalize_enabled) {
			v->normal = glm::normalize(v->normal);
	//	    gl_V3_Norm(&v->normal);
		}
    } 
	else 
	{
		/* no eye coordinates needed, no normal */
		/* NOTE: W = 1 is assumed */
		m = glm::value_ptr(c->matrix_model_projection);

		v->pc.x = (v->coord.x * m[0] + v->coord.y * m[1]) + v->coord.z * m[2] + m[3];
		v->pc.y = (v->coord.x * m[4] + v->coord.y * m[5]) + v->coord.z * m[6] + m[7];
		v->pc.z = (v->coord.x * m[8] + v->coord.y * m[9]) + v->coord.z * m[10] + m[11];

		if (c->matrix_model_projection_no_w_transform) 
		{
			v->pc.w = m[15];
		} 
		else 
		{
			v->pc.w = ((((v->coord.x*m[12])+(v->coord.y* m[13]))+(v->coord.z* m[14]))+m[15]);
		}
    }

    v->clip_code = gl_clipcode(v->pc.x, v->pc.y, v->pc.z, v->pc.w);
}

void glopVertex(GLContext * c, GLParam * p)
{
    GLVertex *v;
    int n, i, cnt;

    assert(c->in_begin != 0);

    n = c->vertex_n;
    cnt = c->vertex_cnt;
    cnt++;
    c->vertex_cnt = cnt;

    /* quick fix to avoid crashes on large polygons */
    if (n >= c->vertex_max) 
	{
		GLVertex *newarray;
		c->vertex_max <<= 1;	/* just double size */
		newarray = (GLVertex *)gl_malloc(sizeof(GLVertex) * c->vertex_max);
		if (!newarray) 
		{
		    gl_fatal_error("unable to allocate GLVertex array.\n");
		}
		memcpy(newarray, c->vertex, n * sizeof(GLVertex));
		gl_free(c->vertex);
		c->vertex = newarray;
    }
    /* new vertex entry */
    v = &c->vertex[n];
    n++;

    v->coord.x = p[1].f;
    v->coord.y = p[2].f;
    v->coord.z = p[3].f;
    v->coord.w = p[4].f;

    gl_vertex_transform(c, v);

    /* color */

    if (c->lighting_enabled) 
	{
		gl_shade_vertex(c, v);
    } 
	else 
	{
		v->color = c->current_color;
    }

    /* tex coords */

    if (c->texture_2d_enabled) 
	{
		if (c->apply_texture_matrix) 
		{
			v->tex_coord = c->matrix_stack_ptr[2][0] * c->current_tex_coord;
//	    gl_M4_MulV4(&v->tex_coord, c->matrix_stack_ptr[2], &c->current_tex_coord);
		} 
		else 
		{
	    v->tex_coord = c->current_tex_coord;
		}
    }
    /* precompute the mapping to the viewport */
    if (v->clip_code == 0)
		gl_transform_to_viewport(c, v);

    /* edge flag */

    v->edge_flag = c->current_edge_flag;

    switch (c->begin_type) 
	{
    case GL_POINTS:
		gl_draw_point(c, &c->vertex[0]);
		n = 0;
		break;

    case GL_LINES:
		if (n == 2) 
		{
		    gl_draw_line(c, &c->vertex[0], &c->vertex[1]);
			n = 0;
		}
		break;
    case GL_LINE_STRIP:
    case GL_LINE_LOOP:
		if (n == 1) 
		{
			c->vertex[2] = c->vertex[0];
		} 
		else 
		if (n == 2) 
		{
		    gl_draw_line(c, &c->vertex[0], &c->vertex[1]);
			c->vertex[0] = c->vertex[1];
			n = 1;
		}
		break;
    case GL_TRIANGLES:
		if (n == 3) 
		{
			gl_draw_triangle(c, &c->vertex[0], &c->vertex[1], &c->vertex[2]);
			n = 0;
		}
		break;
    case GL_TRIANGLE_STRIP:
		if (cnt >= 3) 
		{
			if (n == 3)
				n = 0;
            /* needed to respect triangle orientation */
            switch(cnt & 1) 
			{
            case 0:
      			gl_draw_triangle(c,&c->vertex[2],&c->vertex[1],&c->vertex[0]);
      			break;

				default:
//				case 1:
      			gl_draw_triangle(c,&c->vertex[0],&c->vertex[1],&c->vertex[2]);
      			break;
            }
		}
		break;
    case GL_TRIANGLE_FAN:
		if (n == 3) 
		{
			gl_draw_triangle(c, &c->vertex[0], &c->vertex[1], &c->vertex[2]);
			c->vertex[1] = c->vertex[2];
			n = 2;
		}
		break;

    case GL_QUADS:
		if (n == 4) 
		{
		    c->vertex[2].edge_flag = 0;
			gl_draw_triangle(c, &c->vertex[0], &c->vertex[1], &c->vertex[2]);
			c->vertex[2].edge_flag = 1;
			c->vertex[0].edge_flag = 0;
			gl_draw_triangle(c, &c->vertex[0], &c->vertex[2], &c->vertex[3]);
			n = 0;
		}
		break;

    case GL_QUAD_STRIP:
		if (n == 4) 
		{
			gl_draw_triangle(c, &c->vertex[0], &c->vertex[1], &c->vertex[2]);
			gl_draw_triangle(c, &c->vertex[1], &c->vertex[3], &c->vertex[2]);
			for (i = 0; i < 2; i++)
				c->vertex[i] = c->vertex[i + 2];
			n = 2;
		}
		break;
    case GL_POLYGON:
		break;
		default:
		gl_fatal_error("glBegin: type %x not handled\n", c->begin_type);
    }

    c->vertex_n = n;
}

void glopEnd(GLContext * c, GLParam * param)
{
    assert(c->in_begin == 1);

    if (c->begin_type == GL_LINE_LOOP) 
	{
		if (c->vertex_cnt >= 3) 
		{
			gl_draw_line(c, &c->vertex[0], &c->vertex[2]);
		}
	} 
	else 
	if (c->begin_type == GL_POLYGON) 
	{
		int i = c->vertex_cnt;
		while (i >= 3) 
		{
		    i--;
			gl_draw_triangle(c, &c->vertex[i], &c->vertex[0], &c->vertex[i - 1]);
		}
    }
    c->in_begin = 0;
}
