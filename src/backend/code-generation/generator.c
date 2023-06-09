#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include "../support/logger.h"
#include "../support/garbage-collector.h"
#include "../semantic-analysis/symbol-table.h"
#include "generator.h"

#define SOURCE_FILENAME "imgine_generator.py"

void GenerateExpression(Expression expression);
void GenerateSentence(Sentence sentence);
char * GenerateImagevar(Imagevar imagevar);
void GenerateImagedef(Imagedef imagedef);
void GenerateFiltervar(Filtervar filtervar);
void GenerateFilterdef(Filterdef filterdef);
void GenerateParametersdef(Parametersdef parametersdef);
void GenerateSetvar(Setvar setvar);
void GenerateSetdef(Setdef setdef);
void GenerateImages(Images images);
void GenerateFordef(Fordef fordef);
void GenerateBlock(Block block);
void GenerateFunctions(Functions functions);
void GenerateFilters(Filters filters, char * image);

FILE * file;

int for_count = 0;
int images_inside_for_count = 0;

void Generate(Program program) {
	LogDebug("GenerateProgram");

	file = fopen(SOURCE_FILENAME, "w");

	fprintf(file, "from PIL import Image, ImageEnhance, ImageFilter\nimport numpy as np\nimport os\n\nclass Filter:\n\tblack_threshold = 100\n\n\tdef __init__(self):\n\t\tself.filters = {\n\t\t\t\"exposure\": None,\n\t\t\t\"luminosity\": None,\n\t\t\t\"shadows\": None,\n\t\t\t\"contrast\": None,\n\t\t\t\"brightness\": None,\n\t\t\t\"saturation\": None,\n\t\t\t\"opacity\": None\n\t\t}\n\n\tdef add(self, filter, factor):\n\t\tself.filters[filter] = factor\n\t\treturn self\n\n\tdef apply(self, image):\n\t\tif self.filters.get(\"exposure\") is not None:\n\t\t\timage = ImageEnhance.Brightness(image).enhance(self.filters.get(\"exposure\"))\n\n\t\tif self.filters.get(\"luminosity\") is not None:\n\t\t\timage = ImageEnhance.Brightness(image).enhance(self.filters.get(\"luminosity\"))\n\n\t\tif self.filters.get(\"shadows\") is not None:\n\t\t\timage_array = np.array(image)\n\t\t\tfor row in image_array:\n\t\t\t\tfor pixel in row:\n\t\t\t\t\tif (0.2989 * pixel[0]) + (0.5870 * pixel[1]) + (0.1140 * pixel[2]) <= self.black_threshold:\n\t\t\t\t\t\tfactor_aux = self.filters.get(\"shadows\") - 1\n\t\t\t\t\t\tif pixel[0] + 100 * factor_aux > 255:\n\t\t\t\t\t\t\tpixel[0] = 255\n\t\t\t\t\t\telif pixel[0] + 100 * factor_aux < 0:\n\t\t\t\t\t\t\tpixel[0] = 0\n\t\t\t\t\t\telse:\n\t\t\t\t\t\t\tpixel[0] += 100 * factor_aux\n\n\t\t\t\t\t\tif pixel[1] + 100 * factor_aux > 255:\n\t\t\t\t\t\t\tpixel[1] = 255\n\t\t\t\t\t\telif pixel[1] + 100 * factor_aux < 0:\n\t\t\t\t\t\t\tpixel[1] = 0\n\t\t\t\t\t\telse:\n\t\t\t\t\t\t\tpixel[1] += 100 * factor_aux\n\n\t\t\t\t\t\tif pixel[2] + 100 * factor_aux > 255:\n\t\t\t\t\t\t\tpixel[2] = 255\n\t\t\t\t\t\telif pixel[2] + 100 * factor_aux < 0:\n\t\t\t\t\t\t\tpixel[2] = 0\n\t\t\t\t\t\telse:\n\t\t\t\t\t\t\tpixel[2] += 100 * factor_aux\n\t\t\timage = Image.fromarray(image_array)\n\n\t\tif self.filters.get(\"contrast\") is not None:\n\t\t\timage = ImageEnhance.Contrast(image).enhance(self.filters.get(\"contrast\"))\n\n\t\tif self.filters.get(\"brightness\") is not None:\n\t\t\timage = ImageEnhance.Brightness(image).enhance(self.filters.get(\"brightness\"))\n\n\t\tif self.filters.get(\"saturation\") is not None:\n\t\t\timage = ImageEnhance.Color(image).enhance(self.filters.get(\"saturation\"))\n\n\t\tif self.filters.get(\"opacity\") is not None:\n\t\t\tfactor = self.filters.get(\"opacity\")\n\t\t\topacity = factor if factor <= 1 else 1\n\t\t\timage = image.convert(\"RGBA\")\n\t\t\talpha = image.split()[3]\n\t\t\talpha = alpha.point(lambda p: p * opacity)\n\t\t\timage.putalpha(alpha)\n\t\t\n\t\treturn image\n\nclass NamedFilter:\n\tdef __init__(self, filter):\n\t\tself.filter = filter\n\n\tdef apply(self, image):\n\t\timage = image.filter(self.filter)\n\t\treturn image\n\nif not os.path.exists(\"./imgine_output\"):\n\tos.mkdir(\"imgine_output\")\n\n");

	GenerateExpression(program->expression);

	fclose(file);

	if (!state.generate_images) {
		return;
	}

	pid_t pid = fork();
	if (pid == 0) {
		char * args[] = {"/usr/bin/python3", SOURCE_FILENAME, NULL};
		execv(args[0], args);
	} else if (pid == -1) {
		LogError("Error al crear proceso para generar imágenes.");
	} else {
		int status;
		LogInfo("Generando imágenes...");
		waitpid(pid, &status, 0);
		if (WIFSIGNALED(status)) {
			LogError("Error al generar las imágenes con Python");
		}
		else if (WEXITSTATUS(status) == 0) {
			LogInfo("Imágenes generadas");
		}
	}
}

void GenerateExpression(Expression expression) {
	LogDebug("GenerateExpression");

	switch (expression->type) {
		case EXPRESSIONTYPE_IMAGEDEF:
			GenerateImagedef(expression->imagedef);
			GenerateSentence(expression->sentence);
			break;
		case EXPRESSIONTYPE_FILTERDEF:
			GenerateFilterdef(expression->filterdef);
			GenerateExpression(expression->expression);
			break;
		case EXPRESSIONTYPE_SETDEF:
			GenerateSetdef(expression->setdef);
			GenerateExpression(expression->expression);
			break;
		case EXPRESSIONTYPE_FORDEF:
			GenerateFordef(expression->fordef);
			GenerateExpression(expression->expression);
			break;
	}
}

void GenerateSentence(Sentence sentence) {
	LogDebug("GenerateSentence");

	switch (sentence->type) {
		case SENTENCETYPE_IMAGEDEF:
			GenerateImagedef(sentence->imagedef);
			GenerateSentence(sentence->sentence);
			break;
		case SENTENCETYPE_FILTERDEF:
			GenerateFilterdef(sentence->filterdef);
			GenerateSentence(sentence->sentence);
			break;
		case SENTENCETYPE_SETDEF:
			GenerateSetdef(sentence->setdef);
			GenerateSentence(sentence->sentence);
			break;
		case SENTENCETYPE_FORDEF:
			GenerateFordef(sentence->fordef);
			GenerateSentence(sentence->sentence);
			break;
		case SENTENCETYPE_FUNCTIONS:
			GenerateFunctions(sentence->functions);
			GenerateSentence(sentence->sentence);
			break;
	}
}

char * GenerateImagevar(Imagevar imagevar) {
	LogDebug("GenerateImagevar");

	char * answer;
	switch (imagevar->type) {
		case IMAGEVARTYPE_PATH:
			answer = Malloc(30 + strlen(imagevar->path) + 1);
			strcpy(answer, "Image.open(\"");
			strcpy(&answer[12], imagevar->path);
			strcpy(&answer[12 + strlen(imagevar->path)], "\").convert(\"RGBA\")");
			answer[30 + strlen(imagevar->path)] = '\0';
			break;
		case IMAGEVARTYPE_VAR_NAME:
			answer = Malloc(strlen(imagevar->var_name) + 1);
			strcpy(answer, imagevar->var_name);
			answer[strlen(imagevar->var_name)] = '\0';
			break;
	}
	return answer;
}

void GenerateImagedef(Imagedef imagedef) {
	LogDebug("GenerateImagedef");

	fprintf(file, "%s = %s\n", imagedef->var_name, GenerateImagevar(imagedef->imagevar));
}

void GenerateFiltervar(Filtervar filtervar) {
	LogDebug("GenerateFiltervar");

	switch (filtervar->type) {
		case FILTERVARTYPE_PARAMETERSDEF:
			fprintf(file, "Filter()");
			GenerateParametersdef(filtervar->parametersdef);
			break;
		case FILTERVARTYPE_FILTER_NAME:
			fprintf(file, "NamedFilter(ImageFilter.%s)", filtervar->filter_name);
			break;
		case FILTERVARTYPE_VAR_NAME:
			fprintf(file, "%s", filtervar->var_name);
			break;
	}
}

void GenerateFilterdef(Filterdef filterdef) {
	LogDebug("GenerateFilterdef");

	fprintf(file, "%s = ", filterdef->var_name);
	GenerateFiltervar(filterdef->filtervar);
	fprintf(file, "\n");
}

void GenerateParameter(Property property, float value) {
	LogDebug("GenerateParameter");

	char property_str[11] = "";
	switch (property) {
		case PROPERTY_EXPOSURE:
			strcpy(property_str, "exposure");
			break;
		case PROPERTY_LUMINOSITY:
			strcpy(property_str, "luminosity");
			break;
		case PROPERTY_SHADOWS:
			strcpy(property_str, "shadows");
			break;
		case PROPERTY_CONTRAST:
			strcpy(property_str, "contrast");
			break;
		case PROPERTY_BRIGHTNESS:
			strcpy(property_str, "brightness");
			break;
		case PROPERTY_SATURATION:
			strcpy(property_str, "saturation");
			break;
		case PROPERTY_OPACITY:
			strcpy(property_str, "opacity");
			break;
	}
	fprintf(file, ".add(\"%s\", %.2f)", property_str, value);
}

void GenerateParametersdef(Parametersdef parametersdef) {
	LogDebug("GenerateParametersdef");

	switch (parametersdef->type) {
		case PARAMETERSDEFTYPE_PARAMETER:
			GenerateParameter(parametersdef->property->property, parametersdef->value);
			GenerateParametersdef(parametersdef->parametersdef);
			break;
		case PARAMETERSDEFTYPE_LAST:
			GenerateParameter(parametersdef->property->property, parametersdef->value);
			break;
	}
}

void GenerateSetvar(Setvar setvar) {
	LogDebug("GenerateSetvar");

	switch (setvar->type) {
		case SETVARTYPE_VAR_NAME:
			fprintf(file, "%s", setvar->var_name);
			break;
		case SETVARTYPE_SET:
			fprintf(file, "[");
			GenerateImages(setvar->images);
			fprintf(file, "]");
			break;
	}
}

void GenerateSetdef(Setdef setdef) {
	LogDebug("GenerateSetdef");

	fprintf(file, "%s = ", setdef->var_name);
	GenerateSetvar(setdef->setvar);
	fprintf(file, "\n");
}

void GenerateImages(Images images) {
	LogDebug("GenerateImages");

	switch (images->type) {
		case IMAGESTYPE_SINGLE:
			fprintf(file, "%s", GenerateImagevar(images->imagevar));
			break;
		case IMAGESTYPE_MULTIPLE:
			fprintf(file, "%s, ", GenerateImagevar(images->imagevar));
			GenerateImages(images->images);
			break;
	}
}

void GenerateFordef(Fordef fordef) {
	LogDebug("GenerateFordef");

	for_count++;
	fprintf(file, "if not os.path.exists(\"./imgine_output/bulk_edit_%u\"):\n\tos.mkdir(\"imgine_output/bulk_edit_%u\")\n", for_count, for_count);
	fprintf(file, "for _imgine_index, %s in enumerate(", fordef->forvar->var_name);
	GenerateSetvar(fordef->setvar);
	fprintf(file, "):\n");
	GenerateBlock(fordef->block);
}

void GenerateBlock(Block block) {
	LogDebug("GenerateBlock");

	switch (block->type) {
		case BLOCKTYPE_SINGLE:
			fprintf(file, "\t");
			GenerateFunctions(block->functions);
			break;
		case BLOCKTYPE_MULTIPLE:
			fprintf(file, "\t");
			GenerateFunctions(block->functions);
			GenerateBlock(block->block);
			break;
	}
}

void GenerateFunctions(Functions functions) {
	LogDebug("GenerateFunctions");

	char * imagevar;
	switch (functions->type) {
		case FUNCTIONSTYPE_APPLY_FILTERS:
			GenerateFilters(functions->filters, functions->var_name);
			break;
		case FUNCTIONSTYPE_OVERLAP:
			imagevar = GenerateImagevar(functions->imagevar);
			fprintf(file, "%s.paste(%s, (", functions->var_name, imagevar);
			switch (functions->position->position) {
				case POSITION_TOP_LEFT:
					fprintf(file, "0, 0");
					break;
				case POSITION_TOP_CENTER:
					fprintf(file, "%s.width // 2 - %s.width // 2, 0", functions->var_name, imagevar);
					break;
				case POSITION_TOP_RIGHT:
					fprintf(file, "%s.width - %s.width, 0", functions->var_name, imagevar);
					break;
				case POSITION_CENTER_LEFT:
					fprintf(file, "0, %s.height // 2 - %s.height // 2", functions->var_name, imagevar);
					break;
				case POSITION_CENTER_CENTER:
					fprintf(file, "%s.width // 2 - %s.width // 2, %s.height // 2 - %s.height // 2", functions->var_name, imagevar, functions->var_name, imagevar);
					break;
				case POSITION_CENTER_RIGHT:
					fprintf(file, "%s.width - %s.width, %s.height // 2 - %s.height // 2", functions->var_name, imagevar, functions->var_name, imagevar);
					break;
				case POSITION_BOTTOM_LEFT:
					fprintf(file, "0, %s.height - %s.height", functions->var_name, imagevar);
					break;
				case POSITION_BOTTOM_CENTER:
					fprintf(file, "%s.width // 2 - %s.width // 2, %s.height - %s.height", functions->var_name, imagevar, functions->var_name, imagevar);
					break;
				case POSITION_BOTTOM_RIGHT:
					fprintf(file, "%s.width - %s.width, %s.height - %s.height", functions->var_name, imagevar, functions->var_name, imagevar);
					break;
			}			
			fprintf(file, "), %s.convert(\"RGBA\"))", imagevar);
			break;
		case FUNCTIONSTYPE_RESIZE:
			fprintf(file, "%s = %s.resize((%d, %d))", functions->var_name, functions->var_name, (int) functions->width, (int) functions->height);
			break;
		case FUNCTIONSTYPE_UNION:
			imagevar = GenerateImagevar(functions->imagevar);
			switch (functions->axis->axis) {
				case AXIS_X:
					fprintf(file, "_imgine_aux = Image.new(\"RGB\", (%s.width + %s.width, max(%s.height, %s.height)), (0,0,0)); _imgine_aux.paste(%s, (0, 0)); _imgine_aux.paste(%s, (%s.width, 0)); %s = _imgine_aux.copy()", functions->var_name, imagevar, functions->var_name, imagevar, functions->var_name, imagevar, functions->var_name, functions->var_name);
					break;
				case AXIS_Y:
					fprintf(file, "_imgine_aux = Image.new(\"RGB\", (max(%s.width, %s.width), %s.height + %s.height), (0,0,0)); _imgine_aux.paste(%s, (0, 0)); _imgine_aux.paste(%s, (0, %s.height)); %s = _imgine_aux.copy()", functions->var_name, imagevar, functions->var_name, imagevar, functions->var_name, imagevar, functions->var_name, functions->var_name);
					break;
			}
			break;
		case FUNCTIONSTYPE_TRIM:
			fprintf(file, "%s = %s.crop((", functions->var_name, functions->var_name);
			switch (functions->position->position) {
				case POSITION_TOP_LEFT:
					fprintf(file, "0, 0, %d, %d", (int) functions->width, (int) functions->height);
					break;
				case POSITION_TOP_CENTER:
					fprintf(file, "%s.width // 2 - %d // 2, 0, %s.width // 2 + %d // 2, %d", functions->var_name, (int) functions->width, functions->var_name, (int) functions->width, (int) functions->height);
					break;
				case POSITION_TOP_RIGHT:
					fprintf(file, "%s.width - %d, 0, %s.width, %d", functions->var_name, (int) functions->width, functions->var_name, (int) functions->height);
					break;
				case POSITION_CENTER_LEFT:
					fprintf(file, "0, %s.height // 2 - %d // 2, %d, %s.height // 2 + %d // 2", functions->var_name, (int) functions->height, (int) functions->width, functions->var_name, (int) functions->height);
					break;
				case POSITION_CENTER_CENTER:
					fprintf(file, "%s.width // 2 - %d // 2, %s.height // 2 - %d // 2, %s.width // 2 + %d // 2, %s.height // 2 + %d // 2", functions->var_name, (int) functions->width, functions->var_name, (int) functions->height, functions->var_name, (int) functions->width, functions->var_name, (int) functions->height);
					break;
				case POSITION_CENTER_RIGHT:
					fprintf(file, "%s.width - %d, %s.height // 2 - %d // 2, %s.width, %s.height // 2 + %d // 2", functions->var_name, (int) functions->width, functions->var_name, (int) functions->height, functions->var_name, functions->var_name, (int) functions->height);
					break;
				case POSITION_BOTTOM_LEFT:
					fprintf(file, "0, %s.height - %d, %d, %s.height", functions->var_name, (int) functions->height, (int) functions->width, functions->var_name);
					break;
				case POSITION_BOTTOM_CENTER:
					fprintf(file, "%s.width // 2 - %d // 2, %s.height - %d, %s.width // 2 + %d // 2, %s.height", functions->var_name, (int) functions->width, functions->var_name, (int) functions->height, functions->var_name, (int) functions->width, functions->var_name);
					break;
				case POSITION_BOTTOM_RIGHT:
					fprintf(file, "%s.width - %d, %s.height - %d, %s.width, %s.height", functions->var_name, (int) functions->width, functions->var_name, (int) functions->height, functions->var_name, functions->var_name);
					break;
			}			
			fprintf(file, "))");
			break;
		case FUNCTIONSTYPE_SAVE:
			Symbol image = GetFromSymbolTable(functions->var_name);
			if (image->type == VARTYPE_IMAGE) {
				fprintf(file, "%s.save(os.path.join(\".\", \"imgine_output\", os.path.splitext(os.path.basename(\"%s\"))[0] + \".png\"))", functions->var_name, image->path);
			} else if (image->type == VARTYPE_FOR_IMAGE) {
				fprintf(file, "%s.save(os.path.join(\".\", \"imgine_output\", \"bulk_edit_%u\", str(_imgine_index) + \".png\"))", functions->var_name, for_count);
			}
			break;
	}
	fprintf(file, "\n");
}

void GenerateFilters(Filters filters, char * image) {
	LogDebug("GenerateFilters");

	switch (filters->type) {
		case FILTERSTYPE_SINGLE:
			fprintf(file, "%s = ", image);
			GenerateFiltervar(filters->filtervar);
			fprintf(file, ".apply(%s)", image);
			break;
		case FILTERSTYPE_MULTIPLE:
			fprintf(file, "%s = ", image);
			GenerateFiltervar(filters->filtervar);
			fprintf(file, ".apply(%s); ", image);
			GenerateFilters(filters->filters, image);
			break;
	}
}
