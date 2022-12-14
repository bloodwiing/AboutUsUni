# Library

A tool to replace placeholders based on a specific syntax in the file's contents.

**IMPORTANT!**<br>
This explains every aspect of the library in _detail_.<br>
For the tech-savvy developers, it is recommended to check out and play around with the [example](#example) and to only check other sections if questions arise.

## Table of Contents
* [Terms](#terms)
* [Concept](#concept)
* [Syntax](#syntax)
* [Escaping](#escaping)
* [Usage](#usage)
* [Example](#example)
* [Technical](#technical)

### Terms
* **Placeholder** - a specified location within a **Dynamic file** where **dynamic** data can be inserted during execution.
* **Static** - non-changing data, such as text that will always be the same.
* **Dynamic** - data that can be of any value, that changes and depends on the execution.
* **Dynamic file** - a data file that stores **static** text and Placeholders.
* **Dynamic Element** - a C struct that stores all static and Placeholder data about a file.

### Concept
The idea was to make a library that would process and fill in the named spaces in the template file via a specific syntax.<br>

### Syntax
Everything in the file, such as XML/HTML tags, strings, special symbols will all just be interpreted as the static content of the file.

Anything that is contained within curly brackets `{}` will be called a **Placeholder**.

A Placeholder in this case is the instruction where the user could put data via their own C code.

Every Placeholder needs to have a name, which is written inside the curly brackets (for example: `{role}` would be a Placeholder with the name _"role"_)<br>
Multiple Placeholders can share the same name, such as in the case where some data should repeat and be identical in the file.<br>
So an example like `{company} - About {company}` would be a valid dynamic file.

### Escaping
In the case where we want to just have a curly bracket (`{` or `}`) as the **static** content, we use a special [escape sequence](#escaping).

By using a double bracket syntax (such as `{{` or `}}`) the library will interpret it as a single static bracket and won't make a Placeholder in that location.

****

As example, by giving the input:
```
This is my {good} {{Test}}.
```
The output of the library would be
```
This is my  {Test}.
```
where `good` is the only Placeholder.

****

Escaped placeholders do not have to go in a pair.

Input such as
```
Escaping: }} }}}}
Escaping: {{ {{{{
```
would just be converted into
```
Escaping: } }}
Escaping: { {{
```

### Usage
1. Initialise the **Dynamic Element**

To use the library you first need to open the **Dynamic file** and pass it to the `createDynamicDocument(File *)` function.

The output of this function will create a `DynaElement *` struct that will store the **static** string and will remember the locations of every Placeholder it was able to find within that file.

2. Fill in Placeholder values of the **Dynamic Element**

For every Placeholder that has a unique name, you will need to give it a replacement value.
> _Note: `{name}` and `{name}` are 2 Placeholders that share the same name, you don't need to give the value for "name" twice_

To give a replacement value, use the `putDynamicData(DynaElement *element, char *key, char *format, ...)` function.<br>
* The `element` is the **Dynamic Element** where you'll replace a Placeholder;
* `key` is the name of the Placeholder, the identifier of which placeholder you'll edit;
* `format` is the [format specifier](https://www.geeksforgeeks.org/format-specifiers-in-c/), it uses the same syntax as any other format-based function such as scanf, printf and is used to make setting numbers as value more customisable;
* `...` is the rest of the arguments that will be used in the format, any extra data like integers, floats, etc.

3. Generate the resulting string

To insert every single replacement value into where each Placeholder was and to get the result as a final string, use the `generateDynamicString(DynaElement *element)` function.

The function itself returns the final string and saves it in the **Dynamic Element** struct as the `result` property.

4. De-allocate used memory by the **Dynamic Element**

To finish your work simply use the `freeDynamicElement(DynaElement *element)`, this will clear every string, dynamic array that was created by the **Dynamic Element** and will free up the element itself.

### Example

Dynamic File / Template:
```html
<html>
<body>
<h1>{title}</h1>
<p>This is {description}?</p>
</body>
</html>
```

C code:
```c
FILE *file = fopen("template.html", "r");
DynaElement *document = createDynamicDocument(file);
fclose(file);

putDynamicData(document, "title", "My Site");
putDynamicData(document, "description", "a float: %.2f", 1.0/3);

printf("%s", generateDynamicString(document));

freeDynamicElement(document);  // remember to always free up memory that is no longer in use
```

Console output:
```html
<html>
<body>
<h1>My Site</h1>
<p>This is a float: 0.33?</p>
</body>
</html>
```

***

## Technical

> This section is purely to explain how the library functions, and what it does in the background.<br>
> For general usage, please refer to the [Usage](#usage) and [Example](#example) sections.

The goal of the library is to have a fast function that could be reused to fill out a template.
So most of the technical steps are part of optimisation.

#### `DynaElement * createDynamicDocument(File *)`
Right after it reads and saves all contents of the given file, the contents get processed.

A new string is allocated of the same size as the original (since it will never exceed the original as text can only shrink from this step)

It loops over every single text character (until it reaches the end of the string) checking for if it is one of the curly brackets.<br>
In the case that it is able to find brackets, it will do the following based on the state:

| State\Symbol            | `{`                                                                                             | `}`                                                                                                                                   |
|-------------------------|-------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------|
| **Scanning**            | Copy everything since last placeholder up to this bracket;<br>Change to **Reading placeholder** | Check if it's followed by another `}`, if so skip ahead by 1;<br>Escape `}`;<br>Increment `offset` by the size of the placeholder + 2 |
| **Reading placeholder** | Escape `{`;<br>Change to **Scanning**;<br>Increment `offset` by 1                               | Save placeholder and location of last placeholder                                                                                     |

By the end of execution, copy the remaining string since the last placeholder's position.

Every placeholder that gets saved gets placed into the placeholder dynamic array.

The result is a string that had every double bracket [escaped](#escaping) and all placeholders removed from it.<br>
This string is saved as the new "original" string - we call this "de-formatting".

Since the final string changes in size because elements get removed, an `offset` is tracked.<br>
It remembers how many characters have been removed, so that when the position of the placeholder is recorded, it gets subtracted by the offset beforehand to account for this.

After the string is processed a new [Hash Map](#https://en.wikipedia.org/wiki/Hash_table) is created, which will be explained why soon.

Finally, the pointer to the `DynaElement` is returned.

> For the most part, you do not need to manually access and change fields stored in the `DynaElement`, everything is already done via provided functions.

***

#### `void putDynamicData(DynaElement *element, char *key, char *format, ...)`
To give an instruction of which placeholder to replace with what value, this function is used.

There is not anything that complicated happening here. But here are steps:
1. Create a new `DynaKey` (struct to hold a replacement value)
2. Save the `key` string in the `DynaKey`
3. Calculate the size of the string after all formatting is done (done via the built-in function `vsnprintf()` from `<stdio.h>`)
4. Allocate the "value" string in the `DynaKey` based on the calculated size
5. Set the value string via the same built-in function `vsnprintf()` from `<stdio.h>`
6. Store the `DynaKey` into the [Hash Map](#https://en.wikipedia.org/wiki/Hash_table) of the `DynaElement` based on the `key`.

**Why a Hash Map?**<br>
We wanted to ensure fast look-up times and low time-complexity, so instead of choosing the strategy to loop over every element until a key match is found, we decided that using a bucket and hashing based system would be better.

***

#### `char * generateDynamicString(DynaElement *element)`
This is where the magic happens and where every single replacement value in `DynaKey`s and the de-formatted string get combined.

There are 2 loops that run during this function: one to calculate the final size of the string for allocation, the other to copy and concatenate values to the final string.

**Loop A:** getting the final size<br>
* Set `final_size` = the de-formatted string size
* Loop over every Placeholder
  * Check if the [Hash Map](#https://en.wikipedia.org/wiki/Hash_table) contains a `DynaKey` by the Placeholder key
    * If so, add the size of the replacement value stored in the `DynaKey` to the `final_size`
    * If not, do nothing
  
Then the string is allocated based on the `final_size`

**Loop B:** filling the final string in
* Loop over every Placeholder
  * Copy the string from the last placeholder position, up to the current placeholder position 
  * Check if the [Hash Map](#https://en.wikipedia.org/wiki/Hash_table) contains a `DynaKey` by the Placeholder key
    * If so, copy the placeholder replacement value stored in the `DynaKey`
    * If not, do nothing
* Copy the remaining string since the last placeholder position

_In the case there have not been any placeholders found, the default value of the last placeholder position is 0, so in that case it will just run the "remainder" part of the code._

***

#### `void freeDynamicElement(DynaElement *element)`
Because the library allocates a lot of strings and arrays dynamically, memory needs to be cleaned up correctly.

The function first makes sure to free up any strings that were allocated.

Loops over every placeholder, frees up the placeholder values and the placeholders themselves.

Then it uses a pointer-based loop via the [HashMap.c](https://github.com/tidwall/hashmap.c) library's `hashmap_iter()` function to de-allocate every key and replacement value.

And finally the Hash Map itself gets de-allocated along with the whole `DynaElement`.
