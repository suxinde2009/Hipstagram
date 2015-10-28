// simple GIMP-octave interface
// compile with:
// gimptool-2.0 --install hipstagram.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/param.h>

#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#define PLUG_IN_PROC   "hipstagram_artistica"
#define PLUG_IN_BINARY "hipstagram_artistica"

typedef struct
{
	char cinput[PATH_MAX + 1];
	char coutput[PATH_MAX + 1];
	char cscript[PATH_MAX + 1];
}OctaveParams;


typedef struct
{
	gboolean  run;
}OctaveInterface;


/* local function prototypes */
inline gint coord( gint x, gint y, gint k, gint channels, gint width ) 
{
	return channels*( width*y + x ) + k;
};


void reset_default();


static void	query (void);
static void	run   (	const gchar      *name,
			gint              nparams,
			const GimpParam  *param,
			gint             *nreturn_vals,
			GimpParam       **return_vals
		      );

static void	octave_region(	GimpPixelRgn   *srcPTR,
				GimpPixelRgn   *dstPTR,
				gint            bpp,
				gint            x,
				gint            y,
				gint            width,
				gint            height);

static void	octave         (GimpDrawable   *drawable);
static gboolean	octave_dialog  (GimpDrawable   *drawable);
static void	preview_update (GimpPreview    *preview);


/* create a few globals, set default values */
static OctaveParams octave_params =  {
	"" /* default input    */
	"" /* default output   */
	"" /* default script   */
};


/* Setting PLUG_IN_INFO */
const GimpPlugInInfo PLUG_IN_INFO =  {
	NULL,  /* init_proc  */
	NULL,  /* quit_proc  */
	query, /* query_proc */
	run,   /* run_proc   */
};


MAIN();


static void query (void)
{
	static const GimpParamDef args[] =
	{
		{ GIMP_PDB_INT32,    (gchar*)"run-mode",  (gchar*)"The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
		{ GIMP_PDB_IMAGE,    (gchar*)"image",     (gchar*)"Image" },
		{ GIMP_PDB_DRAWABLE, (gchar*)"drawable",  (gchar*)"Drawable to draw on" },
		{ GIMP_PDB_STRING,   (gchar*)"input",     (gchar*)"Filename of the input matrix (nyi)" },
		{ GIMP_PDB_STRING,   (gchar*)"output",    (gchar*)"Filename of the output matrix (nyi)" },
		{ GIMP_PDB_STRING,   (gchar*)"script",    (gchar*)"Filename of the Octave script (nyi)" }
	};

	gimp_install_procedure (PLUG_IN_PROC,
				"Call Gnu Octave from GIMP",
				"Call Gnu Octave from GIMP",
				"Mika & Irini",
				"hipstagram plugin for Gimp",
				"2013",
				"artistica filter",
				"GRAY*, RGB*",
				GIMP_PLUGIN,
				G_N_ELEMENTS (args), 0,
				args, NULL);

	gimp_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Hipstagram/");
}


static void run (const gchar *name, gint nparams, const GimpParam  *param, gint *nreturn_vals, GimpParam **return_vals)
{
	static GimpParam   values[1];
	GimpPDBStatusType  status = GIMP_PDB_SUCCESS;
	GimpDrawable      *drawable;
	GimpRunMode        run_mode;

	// create working directory
	char* home = getenv("HOME");
	char dir[PATH_MAX + 1];
	strcpy ( dir, home );
	strcat ( dir, "/.gimp-octave" );
	mkdir( dir, S_IRWXU );

	run_mode = (GimpRunMode)param[0].data.d_int32;

	*return_vals  = values;
	*nreturn_vals = 1;

	values[0].type          = GIMP_PDB_STATUS;
	values[0].data.d_status = status;

	/*
	* Get drawable information...
	*/
	drawable = gimp_drawable_get (param[2].data.d_drawable);
	gimp_tile_cache_ntiles (2 * MAX (drawable->width  / gimp_tile_width () + 1 ,
					 drawable->height / gimp_tile_height () + 1));

	switch (run_mode)
	{
		case GIMP_RUN_INTERACTIVE:
			gimp_get_data (PLUG_IN_PROC, &octave_params);
			/* Reset default values show preview unmodified */

			/* initialize pixel regions and buffer */
			if (! octave_dialog (drawable))
			return;
			break;

		case GIMP_RUN_NONINTERACTIVE:
			if (nparams != 6)
			{
				status = GIMP_PDB_CALLING_ERROR;
			}
			else
			{
				reset_default();
			}
			break;

		case GIMP_RUN_WITH_LAST_VALS:
			gimp_get_data (PLUG_IN_PROC, &octave_params);
			break;

		default:
			break;
	}

	if (status == GIMP_PDB_SUCCESS)
	{
		drawable = gimp_drawable_get (param[2].data.d_drawable);

		/* here we go */
		octave (drawable);

		gimp_displays_flush ();

		/* set data for next use of filter */
		if (run_mode == GIMP_RUN_INTERACTIVE)
			gimp_set_data (PLUG_IN_PROC, &octave_params, sizeof (OctaveParams));

		gimp_drawable_detach(drawable);
		values[0].data.d_status = status;
	}
}


static void octave (GimpDrawable *drawable)
{
	GimpPixelRgn srcPR, destPR;
	gint         x1, y1, x2, y2;
	gint         x, y, width, height;

	/* initialize pixel regions */
	gimp_pixel_rgn_init (&srcPR, drawable,
				0, 0, drawable->width, drawable->height, FALSE, FALSE);
	gimp_pixel_rgn_init (&destPR, drawable,
				0, 0, drawable->width, drawable->height, TRUE, TRUE);

	/* Get the input */
	gimp_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);

	x=x1;
	y=y1;
	width=x2-x1;
	height=y2-y1;

	/* Run */
	octave_region (&srcPR, &destPR, drawable->bpp, x, y, width, height);

	gimp_drawable_flush (drawable);
	gimp_drawable_merge_shadow (drawable->drawable_id, TRUE);
	gimp_drawable_update (drawable->drawable_id, x1, y1, x2 - x1, y2 - y1);
}


/*
*  Writes the layer to an octave-readable matrix,
*  then start the octave script, which reads,
*  modifies and returns a matrix, which is then
*  read back in by this plugin.
*/
static void octave_region (GimpPixelRgn *srcPR,GimpPixelRgn *destPR,gint bpp,gint x,gint y,gint width,gint height)
{
	guchar *layerIn;
	guchar *layerOut;
	FILE *matrixIn;
	FILE *matrixOut;
	gint i,j,k,num;
	int ret;
	gint rbpp, rwidth, rheight;
	gint error = FALSE;

	layerIn  = g_new( guchar, bpp * width * height );
	layerOut = g_new( guchar, bpp * width * height );
	gimp_pixel_rgn_get_rect( srcPR, layerIn,x, y,width, height );

	/*
	*  GIMP -> MatrixIn
	*/
	if (bpp==1)
	{	// one channel
		matrixIn = fopen (octave_params.cinput,"w");

		if (matrixIn != NULL)
		{
			fputs ( "# Created by Hipstagram\n",	matrixIn );
			fputs ( "# name:  matrixIn\n",		matrixIn );
			fputs ( "# type:  matrix\n",		matrixIn );
			fprintf ( matrixIn, "# rows: %i\n",	height );	// # rows: 2
			fprintf ( matrixIn, "# columns: %i\n",	width );	// # columns: 2

			for ( j = 0; j < height; j++ )
			{
				for ( i = 0; i < width; i++ )
				{
					fprintf ( matrixIn, "%i\t", (gint)layerIn[coord( i, j, 0, 1, width )]);
				}
				fputs ( "\n", matrixIn );
			}
			fclose ( matrixIn );
		} 
		else
		{
			gimp_message("Error: Could not open input matrix.");
			printf( "L%i: Error: Could not open input matrix.", __LINE__);
			error = TRUE;
		}
	}
	else 
	{
		//two or more channels
		matrixIn = fopen( octave_params.cinput, "w" );

		if ( matrixIn != NULL )
		{
			fputs ("# Created by Hipstagram\n",	matrixIn);
			fputs ("# name:  matrixIn\n",		matrixIn);
			fputs ("# type:  matrix\n",		matrixIn);
			fprintf ( matrixIn, "# ndims: %i\n",	bpp);		// # ndims: 3
			fprintf ( matrixIn, "%i %i %i\n", height, width, bpp);

			for ( k = 0; k < bpp; k++ ) 
			{
				for ( i = 0; i < width; i++ ) 
				{
					for ( j = 0; j < height; j++ )
					{
						fprintf ( matrixIn, "%i\n", (gint)layerIn[coord( i, j, k, bpp, width )]);
					}
				}
			}
			fclose (matrixIn);
		}
		else 
		{
			gimp_message("Error: Could not open input matrix.");
			printf( "L%i: Error: Could not open input matrix.", __LINE__);
			error = TRUE;
		}
	}

	/*
	*  MatrixIn -> Octave -> MatrixOut
	*/
	if (!error)
	{
		// run Octave via system call
		char command[PATH_MAX + 1];
		strcpy ( command, "octave --silent " );
		strcat ( command, octave_params.cscript );
		printf( "L%i: %s\n", __LINE__, command );
		ret = system ( command );
	}

	/*
	*  MatrixOut -> GIMP
	*/
	if (!error) 
	{
		if (bpp==1) 
		{
			// one channel
			matrixOut = fopen (octave_params.coutput,"r");
			if (matrixOut != NULL)
			{
				// # name: matrixOut
				// # type: matrix
				// # rows: 2
				// # columns: 2
				char first = '#';
				char line[100001];
				char cols[100001];
				char rows[100001];
				while (first == '#') 
				{
					fgets ( line, 100000, matrixOut );
					first = line[0];
					if (first == '#') 
					{
						strcpy ( rows, cols );
						strcpy ( cols, line );
					}
				}

				sscanf ( cols, "%*s %*s %i", &rwidth );
				sscanf ( rows, "%*s %*s %i", &rheight );
				printf("L%i: width: %i, height: %i, rwidth: %i, rheight %i\n", __LINE__, width, height, rwidth, rheight);

				if (width==rwidth && rheight==height) 
				{
					//check size
					i = 0;
					char* tok;
					tok = strtok (line, " ");
					while (tok != NULL) 
					{
						num = atoi(tok);
						layerOut[coord( i, 0, 0, 1, width )] = num;
						tok = strtok (NULL, " ");
						i++;
					}

					for ( j = 1; j < height; j++ ) 
					{
						fgets ( line, 100000, matrixOut );
						tok = strtok (line, " ");
						i = 0;

						while (tok != NULL) 
						{
							num = atoi(tok);
							layerOut[coord( i, j, 0, 1, width )] = num;
							tok = strtok (NULL, " ");
							i++;
						}
					}
				} 
				else 
				{
					gimp_message("Error: Size of return matrix is not equal to input matrix.");
					error = TRUE;
				}
				fclose (matrixOut);
			} 
			else 
			{
				gimp_message("Error: Could not open return matrix.");
				printf( "L%i: Error: Could not open return matrix.", __LINE__);
				error = TRUE;
			}

		} 
		else 
		{
			//two or more channels
			matrixOut = fopen (octave_params.coutput,"r");
			if (matrixOut != NULL)
			{
				// # name: matrixOut
				// # type: matrix
				// # ndims: 3
				char first = '#';
				char str[101];

				while (first == '#')
				{
					fgets ( str, 100, matrixOut );
					first=str[0];
				}

				sscanf ( str, "%i %i %i", &rheight, &rwidth, &rbpp);
				printf( "L%i: rwidth = %i, rheight = %i, rbpp = %i\n", __LINE__, rheight, rwidth, rbpp);

				if (width==rwidth && rheight==height && rbpp==bpp) 
				{
					//check size
					for ( k = 0; k < bpp; k++ ) 
					{
						for ( i = 0; i < width; i++ ) 
						{
							for ( j = 0; j < height; j++ ) 
							{
								fscanf ( matrixOut, "%i", &num);
								layerOut[coord( i, j, k, bpp, width )] = num;
							}
						}
					}
				}
				else 
				{
					gimp_message("Error: Size of return matrix is not equal to input matrix.");
					printf( "L%i: Error: Size of return matrix is not equal to input matrix.", __LINE__);
					error = TRUE;
				}
				fclose (matrixOut);
			}
			else 
			{
				gimp_message("Error: Could not open return matrix.");
				printf( "L%i: Error: Could not open return matrix.", __LINE__);
				error = TRUE;
			}
		}
	}

	if (error) 
	{
		gimp_pixel_rgn_set_rect( destPR, layerIn, x, y, width, height );
	} 
	else 
	{	// this one should be default
		gimp_pixel_rgn_set_rect( destPR, layerOut, x, y, width, height );
	}

	g_free( layerIn );
	g_free( layerOut );
}


static gboolean octave_dialog (GimpDrawable *drawable)
{
	GtkWidget *dialog;
	GtkWidget *main_vbox;
	GtkWidget *preview;
	GtkWidget *button_update;
	gboolean   run;

	//default entries:
	reset_default();

	gimp_ui_init (PLUG_IN_BINARY, TRUE);

	dialog = gimp_dialog_new("Hipstagram", PLUG_IN_BINARY,
				 NULL, (GtkDialogFlags)0,
				 gimp_standard_help_func, PLUG_IN_PROC,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 GTK_STOCK_OK,     GTK_RESPONSE_OK,
				 NULL);

	gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),GTK_RESPONSE_OK,GTK_RESPONSE_CANCEL,-1);

	gimp_window_set_transient (GTK_WINDOW (dialog));

	main_vbox = gtk_vbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 12);
	gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), main_vbox);
	gtk_widget_show (main_vbox);

	// preview
	preview = gimp_drawable_preview_new (drawable, NULL);
	gtk_box_pack_start (GTK_BOX (main_vbox), preview, TRUE, TRUE, 0);
	gtk_widget_show (preview);

	g_signal_connect(preview, "invalidated", G_CALLBACK (preview_update),NULL);

	// update button
	button_update = gtk_button_new_with_label("Update");
	gtk_box_pack_start (GTK_BOX (main_vbox), button_update, TRUE, TRUE, 0);
	gtk_widget_show (button_update);
	g_signal_connect_swapped (button_update, "released", G_CALLBACK (gimp_preview_invalidate), preview);

	gtk_widget_show (dialog);
	run = (gimp_dialog_run (GIMP_DIALOG (dialog)) == GTK_RESPONSE_OK);
	gtk_widget_destroy (dialog);

	return run;
}


static void preview_update (GimpPreview *preview)
{
	GimpDrawable *drawable;
	gint          x, y;
	gint          width, height;
	GimpPixelRgn  srcPR;
	GimpPixelRgn  destPR;

	drawable = gimp_drawable_preview_get_drawable (GIMP_DRAWABLE_PREVIEW (preview));

	gimp_pixel_rgn_init ( &srcPR, drawable, 0, 0, drawable->width, drawable->height, FALSE, FALSE);
	gimp_pixel_rgn_init (&destPR, drawable, 0, 0, drawable->width, drawable->height, TRUE, TRUE);

	gimp_preview_get_position (preview, &x, &y);
	gimp_preview_get_size (preview, &width, &height);

	octave_region (&srcPR, &destPR, drawable->bpp, x, y, width, height);

	gimp_pixel_rgn_init (&destPR, drawable, x, y, width, height, FALSE, TRUE);
	gimp_drawable_preview_draw_region (GIMP_DRAWABLE_PREVIEW (preview), &destPR);
}


void reset_default() 
{
	char* home = getenv("HOME");

	// set standard input/output/script strings
	strcpy ( octave_params.cinput, home );
	strcat ( octave_params.cinput, "/.gimp-octave/matrixIn.txt" );
	strcpy ( octave_params.coutput, home );
	strcat ( octave_params.coutput, "/.gimp-octave/matrixOut.txt" );
	strcpy ( octave_params.cscript, home );
	strcat ( octave_params.cscript, "/.gimp-octave/artistica.m" );

	// if there is no Octave script, create one:
	int acc = access( octave_params.cscript, R_OK );
	if (acc == -1)
	{
		FILE* pFile = fopen (octave_params.cscript,"w");
		if (pFile!=NULL)
		{
			fputs ("%----Begin of octave script - created by hipstagram.c:--\n"
				"clear;\n"
				"cd ~/.gimp-octave\n"
				"load matrixIn.txt;\n"
				"[rows, cols, bpp] = size(matrixIn)\n"
				"tmp = matrixIn;\n"
				"%-----------------------------------------\n"
				"\n"
				"\n"
				"%----Put your own stuff here:-------------\n"
				"%  1 = phasecong2\n"
				"%  2 = spatialgabor\n"
				"%  3 = phasesym\n"
				"%  4 = gaborconvolve\n"
				"%  5 = monofilt\n"
				"%  6 = edge\n"
				"%  7 = anisodiff\n"
				"%  8 = adjcontrast\n"
				"%  9 = homomorphic\n"
				"% 10 = retro_filter\n"
				"\n"
				"method = 10;\n"
				"\n"
				"switch (method)\n"
				"\n"
				"case 1\n"
				"%tmp = phasecong2(tmp);\n"
				"tmp = fftshift(tmp);\n"
				"\n"
				"case 2\n"
				"% [Eim, Oim, Aim] =  spatialgabor(im, wavelength, angle, kx, ky, showfilter)\n"
				"[a tmp b] = spatialgabor(tmp, 1, 0, 0.5, 0.5, 0);\n"
				"\n"
				"case 3\n"
				"[tmp a b] = phasesym(tmp);\n"
				"\n"
				"case 4\n"
		"% gaborconvolve(im2, nscale, norient, minWaveLength, mult, sigmaOnf, dThetaOnSigma, feedback, scale, orientation)\n"
				"tmp = angle(gaborconvolve(tmp, 4, 6, 3, 2, 0.65, 1.5, 0, 4, 6));\n"
				"\n"
				"case 5\n"
			"% function [f, h1f, h2f, A, theta, psi] = monofilt(im2, nscale, minWaveLength, mult, sigmaOnf, orientWrap)\n"
					"[f, h1f, h2f, A, theta, psi] = monofilt(tmp, 3, 4, 2, 0.65, 0);\n"
					"for t = 1:bpp\n"
				"	tmp(:,:,t) = f{1,1};" " %first cell\n"
				"end\n"
				"\n"
				"case 6\n"
				"tmp6=zeros(rows,cols);\n"
				"if(bpp>1)\n"
					"for t=1:bpp,\n"
						"tmp6 += tmp(:,:,t);\n"
					"end\n"
					"tmp6 = abs(floor(tmp6./bpp)) + 1;\n"
				"else\n"
					"tmp6=tmp;\n"
				"end\n"
				"\n"
				"tmp6 = edge(ind2gray(tmp6, gray (max(tmp6(:)))),'canny', [0.1 0.2], 1);\n"
				"\n"
				"if(bpp>1)\n"
					"tmp=zeros(rows,cols,bpp);\n"
					"for t=1:bpp,\n"
					"tmp(:,:,t)=tmp6;\n"
					"end\n"
				"else\n"
					"tmp=tmp6;\n"
				"end\n"
				"\n"
				"case 7\n"
				"% diff = anisodiff(im, niter, kappa, lambda, option)\n"
				"tmp = anisodiff(tmp, 100, 30, 0.1, 1);\n"
				"\n"
				"case 8\n"
				"% g = adjcontrast(im, gain, cutoff)\n"
				"tmp = adjcontrast(tmp, 10, 0.5);\n"
				"\n"
				"case 9\n"
				"% him = homomorphic(im, boost, CutOff, order, varargin)\n"
					"tmp = homomorphic(tmp, 2, .25, 2, 0, 5);\n"
				"\n"
				"case 10\n"
				"% [ img ] = retro_filter_vectorized( img, alpha, beta, gamma, delta, epsilon )\n"
				"tmp = retro_filter( tmp, 0.1, 0.5, 0.3, 0.5, 0.05);\n"
				"\n"
				"otherwise\n"
					"printf('Wrong method!\n');\n"
				"endswitch\n"
				"\n"
				"\n"
				"%stretch from 0 to 255\n"
				"tmp = tmp .- min(tmp(:));\n"
				"tmp = 255*tmp/(max(tmp(:)));\n"
				"\n"
				"%-----------------------------------------"
				"\n"
				"%----End of octave script, don't edit:----"
				"matrixOut = round(tmp);\n"
				"matrixOut(matrixOut(:)<0)=0;\n"
				"matrixOut(matrixOut(:)>255)=255;\n"
				"dims = ndims (matrixOut)\n"
				"if (dims == 2)\n"
				"matrixOut = resize (matrixOut, [size(matrixOut, 1), size(matrixOut, 2), 1]);\n"
				"end\n"
				"\n"
				"size(matrixOut)\n"
				"save matrixOut.txt matrixOut;\n"
				"%-----------------------------------------\n",pFile);
			fclose (pFile);
		}
	}
}
