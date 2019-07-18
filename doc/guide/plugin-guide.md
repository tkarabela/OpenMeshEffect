
Writing an Open Mesh Effect Plug-in
===================================

In this document, we will use a couple of simple examples of plug-ins to illustrate the main steps involved in writing an Open Mesh Effect plug-in.

The first example is a mirror plug-in. It doubles the geometry, symmetrizing the duplicate. It requires to read some input data, allocate a new mesh for output, while it does not involve complex mathematical operation that could disturb the reader.

This example will be provided in the C programming language, but you can use any other one, as long as you are able to comply with the C include files of the standard. In particular, you can transparently use the C++ language if you prefer.

Getting started
---------------

Create a shared library project with your favorite C tool chain. Call your main source file for instance `mirror_plugin.c`, and copy along [the Open Mesh Effect include files](../../include) to your project.

When using Visual Studio, don't forget to set the binary type to dynamic library (.dll) instead of exe. When using CMake, use `add_library(... SHARED ...)` to create a dynamic library. Set the file extension of the output file to .ofx rather than .dll or .so using `set_target_properties(mirror_plugin PROPERTIES SUFFIX ".ofx")` (there is a similar option with Visual Studio). When using `gcc`, use the `-shared` flag.

Basic structure
---------------

We will write everything in `mirror_plugin.c` for the sake of clarity, but for a more substantial use case you might want to split up your code into several files at some point.

We begin with a generic OpenFX skeleton:

```C
#include "ofxCore.h"
#include "ofxMeshEffect.h"

static void setHost(OfxHost *host) {
}

static OfxStatus mainEntry(const char *action,
                           const void *handle,
                           OfxPropertySetHandle inArgs,
                           OfxPropertySetHandle outArgs) {
}

OfxExport int OfxGetNumberOfPlugins(void) {
    return 1;
}

OfxExport OfxPlugin *OfxGetPlugin(int nth) {
    static OfxPlugin plugin = {
        /* pluginApi */          kOfxMeshEffectPluginApi,
        /* apiVersion */         kOfxMeshEffectPluginApiVersion,
        /* pluginIdentifier */   "MirrorPlugin",
        /* pluginVersionMajor */ 1,
        /* pluginVersionMinor */ 0,
        /* setHost */            setHost,
        /* mainEntry */          mainEntry
    };
    return &plugin;
}
```

Note than even though it is a generic OpenFX boilerplate, we need to include `ofxMeshEffect.h` for the constants `kOfxMeshEffectPluginApi` and `kOfxMeshEffectPluginApiVersion`.

This is a simple example in which we only define one plug-in, but keep in mind that a single OpenFX plug-in binary[°](#gl-plugin-binary) may contain several plug-ins[°](#gl-plugin) at once. This is why the OpenFX standard requires a binary to export the symbols `OfxGetNumberOfPlugins` and `OfxGetPlugin`.

*NB: In `OfxGetPlugin`, you do not have to check that `nth` is a valid index. It is ensured by the standard.*

*NB: The functions that are not exported are marked `static` to mean that they are private.*

Main Entry
----------

The `mainEntry` is called by the host whenever it needs the plug-in to do something. The *something* is specified by the first argument, the `action`. Depending on the action, the remaining arguments will have different meanings.

So the first thing to do in this main entry point is to check which action is requested. Since all actions are different, the behavior for each action will be outsourced in a dedicated function. In the end, the `mainEntry` just contains a long series of conditions such as:

```C
if (0 == strcmp(action, kOfxActionLoad)) {
	return load(handle, inArgs, outArgs);
}
if (0 == strcmp(action, kOfxActionUnload)) {
	return unload(handle, inArgs, outArgs);
}
// ...
```

The `load()`, `unload()`, etc. functions must be defined above. It is important to check the reference for each action, because some of them will ignore some arguments, and most of them will require `handle` to be casted to a more specific type. For instance, the describe action requires it to be a `OfxMeshEffectHandle`.

Actions
-------

For our example, we need to implement the following actions:

 - `kOfxActionLoad`
 - `kOfxActionUnload`
 - `kOfxActionDescribe`
 - `kOfxActionCreateInstance`
 - `kOfxActionDestroyInstance`
 - `kOfxMeshEffectActionCook`

Load and unload actions are called respectively before and after everything else. They don't require any argument.

The describe action is called by the host once to get the overall structure of the plug-in. This is where the plug-in tells what parameters it has, how many inputs it requires, how many outputs it provides, with which attributes, etc. This is fixed and cannot be changed dynamically.

*NB: If you feel that you need to change the number of parameters sometimes, it may be that you should declare several plug-ins in the binary.*

The create and destroy actions are used before and after processing some geometry. You can think of "instances" as nodes. The describe action gets the node type info, and the create/destroy instance actually create and destroy a node. This node my store runtime data that should not be shared with other instances. There can be several instances of the same effect defined, for instance if several objects have a mirror modifier with different settings.

The last action, the *cook*, is introduced by the Mesh Effect API, while the previous actions were part of the more generic OpenFX standard. This action is where the actual, potentially computing intensive, mesh processing occurs.

So, at this point, our mainEntry functions relatives has become:

```C
static OfxStatus load() {
	return kOfxStatReplyDefault;
}

static OfxStatus unload() {
	return kOfxStatReplyDefault;
}

static OfxStatus describe(OfxMeshEffectHandle descriptor) {
	return kOfxStatReplyDefault;
}

static OfxStatus createInstance(OfxMeshEffectHandle instance) {
	return kOfxStatReplyDefault;
}

static OfxStatus destroyInstance(OfxMeshEffectHandle instance) {
	return kOfxStatReplyDefault;
}

static OfxStatus cook(OfxMeshEffectHandle instance) {
	return kOfxStatReplyDefault;
}

static OfxStatus mainEntry(const char *action,
	                       const void *handle,
	                       OfxPropertySetHandle inArgs,
	                       OfxPropertySetHandle outArgs) {
	if (0 == strcmp(action, kOfxActionLoad)) {
		return load();
	}
	if (0 == strcmp(action, kOfxActionUnload)) {
		return unload();
	}
	if (0 == strcmp(action, kOfxActionDescribe)) {
		return describe((OfxMeshEffectHandle)handle);
	}
	if (0 == strcmp(action, kOfxActionCreateInstance)) {
		return createInstance((OfxMeshEffectHandle)handle);
	}
	if (0 == strcmp(action, kOfxActionDestroyInstance)) {
		return destroyInstance((OfxMeshEffectHandle)handle);
	}
	if (0 == strcmp(action, kOfxMeshEffectActionCook)) {
		return cook((OfxMeshEffectHandle)handle);
	}
	return kOfxStatReplyDefault; // this means "unhandled action"
}
```

Suites
------

We must now fill all our action related functions, for instance the `describe()` function to define an input and an "axis" parameter for the user to chose along which axis the mirror is. But what does "defining a parameter" means?

While *actions* are queries from the host to the plug-in, plug-in will ask in return the host to do things. OpenFX provides a mechanism for the host to hand functions to the plug-ins through structures called *suites*.

A suite is a set of functions defined by the host and called from plug-ins, grouped by topic. There is a suite for properties defined in `ofxProperty.h`, one for parameters defined in `ofxParam.h` and one specific to mesh effects in `ofxMeshEffect.h`. A plug-in will ask the host only for the suites it uses. So for instance our plug-in will never ask for the Image Effect suite, used by 2D compositing plug-ins.

The plug-in asks the host for suites using the `fetchSuite` function of an `OfxHost` variable. This is where the `setHost` function we defined at the beginning gets useful. We also need a global variable holding the returned suite.

```C
OfxMeshEffectSuiteV1 *gMeshEffectSuite; // global variable

static void setHost(OfxHost *host) {
	gMeshEffectSuite = host->fetchSuite(
		host->host, // host properties, might be useful to the host's internals
		kOfxMeshEffectSuite, // name of the suite we want to fetch
		1 // version of the suite
	);
}
```

We are ensured by the standard that `setHost` is called before any action, but we need to check that the returned suite is valid. Indeed, a host is not required to implement all the suites. For instance, Natron does not implement the Mesh Effect suite, while on the contrary Blender does not implement the Image Effect suites.

The check is expected to be carried in the `describe` action. If a required suite is found to be null, the special status `kOfxStatErrMissingHostFeature` must be returned.

```C
static void describe(OfxMeshEffectHandle descriptor) {
	if (NULL == gMeshEffectSuite) {
		return kOfxStatErrMissingHostFeature;
	}
	// ...
	return kOfxStatReplyDefault;
}
```

*NB: To prevent the proliferation of global variables when using several suites, which can lead to messy architectures, I like to define only one `gRuntime` global with such a struct:*

```C
typedef struct PluginRuntime {
	OfxHost *host;
	OfxPropertySuiteV1 *propertySuite;
	OfxParameterSuiteV1 *parameterSuite;
	OfxMeshEffectSuiteV1 *meshEffectSuite;
} PluginRuntime;

PluginRuntime gRuntime;
```
*The `gRuntime` can even be passed as a first argument to the action functions, so that only `mainEntry` relies on the global.*

Description
-----------

With these suites in hand, we will focus on how a plug-in describes itself to the host. There are basically two things to handle, namely the inputs/outputs and the parameters.

### Input

We will start with the former. The mesh effect suite provides an `inputDefine` suite. It works like any suite function:

```C
OfxStatus function(Type1 arg1, Type2 arg2, ..., ReturnType *return)
```

The suite function always return a status code, whose set of possible values may vary (see individual functions reference). You should check it, even though to make the code easier for you to read I will omit this here.

The returned value(s) is (are) the last argument, which is a pointer telling where to write the result. This typically points to a local variable, which gives in our case:

```C
OfxPropertySetHandle inputProperties;
meshEffectSuite->inputDefine(descriptor, kOfxMeshMainInput, &inputProperties);
```

The standard describes inputs with *property sets*. These are basically key value stores like what Python calls a dictionary, or what JavaScript calls a JSON object.

Its available fields are standardized, and some hosts may extend it with extra properties. For instance, the `kOfxPropLabel` field is the input label, set by:

```C
propertySuite->propSetString(inputProperties, kOfxPropLabel, 0, "Main Input");
```

With these three lines, we declared that our plug-in has a main input displayed as "Main Input". The identifier `kOfxMeshMainInput` is standardized for hosts such has Blender modifiers that need a main input, other inputs can have arbitrary identifiers.

### Output

I am not sure that it will remain like this, but at the moment outputs are actually also defined as "inputs" in the terms used by the Mesh Effect API. So adding an output is as easy as:

```C
OfxPropertySetHandle outputProps;
meshEffectSuite->inputDefine(descriptor, kOfxMeshMainOutput, &outputProps);
propertySuite->propSetString(outputProps, kOfxPropLabel, 0, "Main Output");
```

### Parameters

The parameters are fields that users can edit, keyframe, etc. They are very different from properties, that are only used internally for host/plug-in interoperability.

The standard for parameters is the same as the one used by the Image Effect API, so you can check out its documentation as well. The effect descriptors (and instances) own an `OfxParamSetHandle` that is retrieved using the `getParamSet` function from the mesh effect suite, and then altered through the parameter suite:

```C
OfxParamSetHandle parameters;
meshEffectSuite->getParamSet(meshEffect, &parameters);
parameterSuite->paramDefine(parameters, kOfxParamTypeDouble, "width", NULL);
parameterSuite->paramDefine(parameters, kOfxParamTypeDouble, "height", NULL);
parameterSuite->paramDefine(parameters, kOfxParamTypeInteger, "axis", NULL);
// ...
```

Cooking
-------

In our example, we do not need to initialize anything when creating an instance, so we can leave aside the `createInstance` and `destroyInstance` functions and focus now on the capital `cook` action.

```C
meshEffectSuite->inputGetHandle(meshEffect, kOfxMeshMainInput, &input, NULL);
```

**WORK IN PROGRESS**

Glossary
--------

*TODO*

### plug-in binary #gl-plugin-binary

A binary file, ending with .ofx though it is technically a .dll or .so file, that contain all the code for one or several plug-ins.

### plug-in #gl-plugin

A single plug-in, defined within a plug-in binary.