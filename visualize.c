#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include <time.h>

#include "gd.h"
#include "gdfontl.h"
#include "gdfontg.h"
#include "fann.h"

/*
	gcc -g -Wall -Wextra visualize.c -o visualize -lgd -lfann
*/


unsigned int array_sum(unsigned int *array, int array_size)
{
	unsigned int sum = 0;
	for (int i=0; i<array_size; i++)
	{
		sum += array[i];
	}
	return sum;
}

unsigned int max_in_array(unsigned int *array, int array_size)
{
	//~unsigned int max = -INT_MAX;
	unsigned int max = 0;
	for(int i=0; i<array_size; i++)
	{
		if (array[i] > max)
			max = array[i];
	}
	return max;
}

int main(int argc, char **argv)
{
	srand(time(0));
	
	int neuron_size = 50; // height & width in px
	//~int neuron_size = 25; // height & width in px
	int tiles_between_neurons = 2;
	
	bool draw_connection_weights = true; //true;
	
	// In densely connected networks it can be difficult to see all the connections so randomizing the 
	// connection color can improve the visibility some, See the Pathfinder examples.
	// true = random color
	// false = colors: red negitive, green positive
	bool random_connection_color = true;//false;
	
	//~int tiles_between_layers = 3;
	int tiles_between_layers = 5;
	
	bool draw_grid = true;
	// Output a second image with the ANN stats?
	bool output_stats_image = true;
	
	// Rotate view?
	// default is "waterfall" inputs on top outputs on bottom rotate set to true is 
	//		inputs on the left and outputs on the right
	bool rotate = true;
	//~bool rotate = false;
	
	// Path to ANN's - change to your bot
	//~char ann_name[] = "xor_float.net";
	//~char ann_name[] = "pathfinder_float.net";
	//~char ann_name[] = "dui.net";
	//~char ann_name[] = "ocr_float.net";
	
	if(argc < 2)
	{
		printf("Error in input\n");
		exit(EXIT_SUCCESS);
	}
	
	//~struct fann *ann = fann_create_from_file(ann_name); //Load ANN
	struct fann *ann = fann_create_from_file(argv[1]); //Load ANN
	if(ann == NULL)
	{
		printf("Error in reading NET file\n");
		exit(EXIT_SUCCESS);
	}
	
	unsigned int num_inputs = fann_get_num_input(ann); // int
	unsigned int num_outputs = fann_get_num_output(ann); // int
	unsigned int total_neurons = fann_get_total_neurons(ann); // int
	//~printf("total_neurons = %d\n", total_neurons);
	
	unsigned int num_layers = fann_get_num_layers(ann); // int
	unsigned int *layers_array = calloc(num_layers, sizeof(unsigned int));
	fann_get_layer_array(ann, layers_array); // array
	
	unsigned int *bias_array = calloc(num_layers, sizeof(unsigned int));
	fann_get_bias_array(ann, bias_array); // array 
	
	//~for (int i=0; i < num_layers; i++)
	//~{
		//~printf("layer[%d] = %d neurons, %d biases\n", i, layers_array[i], bias_array[i]);
	//~}
	
	unsigned int total_connections = fann_get_total_connections(ann); // int
	struct fann_connection *connections_array = malloc(sizeof(struct fann_connection) * total_connections);
	fann_get_connection_array(ann, connections_array); // array of FANN connection objects
	
	int type_of_network = fann_get_network_type(ann);
	//~printf("type of my network: %s \n", FANN_NETTYPE_NAMES[fann_get_network_type(ann)]);
	if (type_of_network == FANN_NETTYPE_LAYER)
	{
		/* Each layer only has connections to the next layer */
		;//
	}
	else if (type_of_network == FANN_NETTYPE_SHORTCUT)
	{
		/* Each layer has connections to all following layers */
		;//
	}
	else
	{
		// error: there are no third type of network in this version of FANN
		printf("Error in Network type\n");
		fann_destroy(ann);
		exit(EXIT_FAILURE);
	}
	
	fann_destroy(ann);
	
	enum type_of_neuron { input, output, hidden, bias};
	
	struct neuron
	{
		unsigned int index;
		unsigned int layer;
		enum type_of_neuron type;
		float x,y;
	};
	
	struct neuron *my_neurons = (struct neuron *)malloc(sizeof(struct neuron) * total_neurons);
	//~struct neuron *my_neurons = (struct neuron *)calloc(total_neurons, sizeof(struct neuron));
	for (unsigned int i=0; i<total_neurons ;i++)
	{
		my_neurons[i].index = i;
	}
	
	// Figure out which neuron belongs on which layer, Assign it a type: ['input', 'output', 'hidden', 'bias']
	unsigned int current_neuron = 0;
	for (unsigned int layer=0; layer < num_layers; layer++)
	{
		// Detect Input, Output & Hidden Neurons 
		unsigned int i;
		for(i = current_neuron; i < (current_neuron + layers_array[layer]); i++)
		{
			my_neurons[i].layer = layer;
			if(layer == 0)
			{
				my_neurons[i].type = input;
			}
			else if(layer == (num_layers - 1))
			{
				my_neurons[i].type = output;    
			}
			else
			{
				my_neurons[i].type = hidden;
			}
		}
		
		current_neuron = i;
		// Detect Bias Neurons 
		for(i = current_neuron; i < (current_neuron + bias_array[layer]); i++)
		{
			my_neurons[i].layer = layer;
			my_neurons[i].type = bias;
		}
		current_neuron = i;
		
		//~printf("layer = %d, i = %d \n", layer, i);
		//~printf("i = %d \n", i);
		assert (i <= total_neurons);
	}
	
	// Determine Neuron X,Y Position
	int row = 1; // y
	int col = 1; // x
	for(unsigned int index=0; index < total_neurons; index++)
	{
		if(index > 0 && my_neurons[index].layer > my_neurons[index - 1].layer)
		{
			row += tiles_between_layers;
			col = 1;
		}
		else
		{
			col += tiles_between_neurons;
		}
		
		my_neurons[index].x = (neuron_size * col) - (neuron_size / 2);
		my_neurons[index].y = (neuron_size * row) - (neuron_size / 2);
		
		//~assert (index <= total_neurons);
	}
	
	//~printf("num_layers = %d \n", num_layers);
	// Get complete_layer count = neurons + bias neurons
	unsigned int *complete_layers = (unsigned int *)calloc(num_layers, sizeof(unsigned int));
	for (unsigned int layer=0; layer<num_layers; layer++)
	{
		complete_layers[layer] = layers_array[layer] + bias_array[layer];
	}
	
	unsigned int largest_layer = max_in_array(complete_layers, num_layers); // Find the largest layer width
	free(complete_layers);
	
	
	// Create a Blank Image
	int image_width = (neuron_size * ((signed int)largest_layer + (tiles_between_neurons / 2))) * tiles_between_neurons - (neuron_size) + 1;
	int image_height = (neuron_size * num_layers) * tiles_between_layers - (neuron_size * (tiles_between_layers - 1)) + 1;
	//~printf("image_width = %d\n", image_width);
	//~printf("image_height = %d\n", image_height);
	gdImagePtr neural_network_image;
	neural_network_image = gdImageCreateTrueColor(image_width, image_height);
	if (!neural_network_image)
	{
		printf("Error in gdImageCreateTrueColor\n");
		exit(EXIT_FAILURE);
	}
	
	// Create Colors Array
	int background_color = gdImageColorAllocate(neural_network_image, 153, 153, 153);
	int grid_color = gdImageColorAllocate(neural_network_image, 128, 128, 128);
	int neuron_stroke_color = gdImageColorAllocate(neural_network_image, 92, 92, 92);
	int input_neuron_color = gdImageColorAllocate(neural_network_image, 170, 255, 170);
	int hidden_neuron_color = gdImageColorAllocate(neural_network_image, 233, 175, 174);
	int output_neuron_color = gdImageColorAllocate(neural_network_image, 171, 204, 255);
	int bias_neuron_color = gdImageColorAllocate(neural_network_image, 255, 255, 0);
	int positive_connection_weight_color = gdImageColorAllocate(neural_network_image, 0, 255, 0);
	int negitive_connection_weight_color = gdImageColorAllocate(neural_network_image, 255, 0, 0);
	int dead_connection_weight_color = gdImageColorAllocate(neural_network_image, 0, 0, 0);
	
	
	// Paint Background
	gdImageFill(neural_network_image, 0, 0, background_color);
	
	// Draw Grid if draw_grid is set to true 
	if(draw_grid == true)
	{
		row = 0;
		col = 0;
		//~foreach(range(0, image_height - 1, 1) as y)
		for (int y=0; y<image_height; y++)
		{
			//~foreach(range(0, image_width  - 1, 1) as x)
			for (int x=0; x<image_width; x++)
			{
				// paint grid
				if(row == neuron_size || ((y % neuron_size) == 0))
				{
					gdImageSetPixel(neural_network_image, x, y, grid_color);
					row = 0;
				}
				if(col == neuron_size || ((x % neuron_size) == 0))
				{
					gdImageSetPixel(neural_network_image, x, y, grid_color);
					col = 0; 
				}
				col++;
			}
			row++;
		}
	}
	
	// Draw Connections
	for (unsigned int i=0; i<total_connections ;i++)
	{
		float weight = connections_array[i].weight;
		int from_neuron = connections_array[i].from_neuron;
		int to_neuron = connections_array[i].to_neuron;
		
		int color;
		// What color is the connection
		if(random_connection_color == false)
		{
			if(weight > 0.0)
			{
				color = positive_connection_weight_color;
			}
			else if(weight < 0.0)
			{
				color = negitive_connection_weight_color;
			}
			else
			{
				color = dead_connection_weight_color;
			}
		}
		else
		{
			color = gdImageColorAllocate(neural_network_image, (int)(rand()*255.0/RAND_MAX), (int)(rand()*255.0/RAND_MAX), (int)(rand()*255.0/RAND_MAX));
		}
		
		// Set connection thickness
		if(draw_connection_weights == true)
		{
			int thickness = 1 + (fabs(weight) * 2);
			//~int thickness = (fabs(weight) * 2);
			if(thickness > 32)
			{
				thickness = 32;
			}
			gdImageSetThickness (neural_network_image , thickness);
		}
		else
		{
			gdImageSetThickness (neural_network_image , 1);
		}
		
		// Draw connection
		gdImageLine(neural_network_image , my_neurons[from_neuron].x, my_neurons[from_neuron].y, my_neurons[to_neuron].x, my_neurons[to_neuron].y, color);
		
		gdImageColorDeallocate(neural_network_image, color);
	}
	
	gdImageSetThickness (neural_network_image, 2); // Reset line brush thickness
	
	// Draw Neurons
	for (unsigned int key=0; key<total_neurons ; key++)
	{
		int color;
		if(my_neurons[key].type == input)
		{
			color = input_neuron_color;
		}
		else if(my_neurons[key].type == hidden)
		{
			color = hidden_neuron_color;
		}
		else if(my_neurons[key].type == output)
		{
			color = output_neuron_color;
		}
		else if(my_neurons[key].type == bias)
		{
			color = bias_neuron_color;
		}
		
		gdImageFilledEllipse(neural_network_image, my_neurons[key].x, my_neurons[key].y, neuron_size, neuron_size, color);
		gdImageArc (neural_network_image, my_neurons[key].x, my_neurons[key].y, neuron_size+1, neuron_size+1, 0, 360, neuron_stroke_color);
	}
	
	// Rotate if you insist on looking at the network wrong! :-P
	if(rotate == 1)
	{
		neural_network_image = gdImageRotateInterpolated(neural_network_image, 90, 0);
		if (neural_network_image == NULL)
		{
			printf("Error in gdImageRotateInterpolated \n");
			exit(EXIT_FAILURE);
		}
	}
	
	// Output the image.
	FILE *png_out = fopen("ann_name.png", "w");
	gdImagePng(neural_network_image, png_out);
	fclose(png_out);
	
	gdImageColorDeallocate(neural_network_image, background_color);
	gdImageColorDeallocate(neural_network_image, grid_color);
	gdImageColorDeallocate(neural_network_image, neuron_stroke_color);
	gdImageColorDeallocate(neural_network_image, input_neuron_color);
	gdImageColorDeallocate(neural_network_image, hidden_neuron_color);
	gdImageColorDeallocate(neural_network_image, output_neuron_color);
	gdImageColorDeallocate(neural_network_image, bias_neuron_color);
	gdImageColorDeallocate(neural_network_image, positive_connection_weight_color);
	gdImageColorDeallocate(neural_network_image, negitive_connection_weight_color);
	gdImageColorDeallocate(neural_network_image, dead_connection_weight_color);
	
	gdImageDestroy(neural_network_image);
	
	
	if(output_stats_image == 1)
	{
		// Create the image
		gdImagePtr neural_network_stats_image = gdImageCreate(250, 400);
		
		// Create Colors Array
		int background_color = gdImageColorAllocate(neural_network_stats_image, 153, 153, 153);
		int inputs_text_color = gdImageColorAllocate(neural_network_stats_image, 170, 255, 170);
		int hidden_text_color = gdImageColorAllocate(neural_network_stats_image, 233, 175, 174);
		int outputs_text_color = gdImageColorAllocate(neural_network_stats_image, 171, 204, 255);
		int bias_text_color = gdImageColorAllocate(neural_network_stats_image, 255, 255, 0);
		int layers_text_color = gdImageColorAllocate(neural_network_stats_image, 0, 128, 128);
		int connections_text_color = gdImageColorAllocate(neural_network_stats_image, 128, 64, 0);
		
		// Paint Background
		gdImageFill(neural_network_stats_image, 0, 0, background_color);

		//~gdFontPtr font = gdFontGetLarge();
		gdFontPtr font = gdFontGetGiant();
		
		int x = 25;
		int y = 25;
		int increment = y + 10;
		
		char tmp_text[100];
		
		sprintf(tmp_text, "%d Inputs", num_inputs);
		gdImageString(neural_network_stats_image, font, x, y, (unsigned char *)tmp_text, inputs_text_color);
		y += increment;
		
		sprintf(tmp_text, "%d Hidden", total_neurons - (num_inputs + num_outputs + array_sum(bias_array, num_layers)));
		gdImageString(neural_network_stats_image, font, x, y, (unsigned char *)tmp_text, hidden_text_color);
		y += increment;
		
		sprintf(tmp_text, "%d Outputs", num_outputs);
		gdImageString(neural_network_stats_image, font, x, y, (unsigned char *)tmp_text, outputs_text_color);
		y += increment;
		
		sprintf(tmp_text, "%d Bias", array_sum(bias_array, num_layers));
		gdImageString(neural_network_stats_image, font, x, y, (unsigned char *)tmp_text, bias_text_color);
		y += increment;
		
		sprintf(tmp_text, "%d Layers", num_layers);
		gdImageString(neural_network_stats_image, font, x, y, (unsigned char *)tmp_text, layers_text_color);
		y += increment;
		
		sprintf(tmp_text, "%d Connections", total_connections);
		gdImageString(neural_network_stats_image, font, x, y, (unsigned char *)tmp_text, connections_text_color);
		y += increment;
		
		FILE *fd_png = fopen("ann_name.stats.png", "w");
		gdImagePng(neural_network_stats_image, fd_png);
		fclose(fd_png);
		
		gdImageColorDeallocate(neural_network_image, background_color);
		gdImageColorDeallocate(neural_network_image, inputs_text_color);
		gdImageColorDeallocate(neural_network_image, hidden_text_color);
		gdImageColorDeallocate(neural_network_image, outputs_text_color);
		gdImageColorDeallocate(neural_network_image, bias_text_color);
		gdImageColorDeallocate(neural_network_image, layers_text_color);
		gdImageColorDeallocate(neural_network_image, connections_text_color);
		
		gdImageDestroy(neural_network_stats_image);
	}
	

	free(my_neurons);
	free(layers_array);
	free(bias_array);
	free(connections_array);
	
}
