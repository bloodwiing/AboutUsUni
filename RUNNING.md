# Running

This page explains how you can use the current "About Us" website template generator to create a similar website as one publicly displayed on GitHub pages: https://bloodwiing.github.io/AboutUsUni/

## Downloading
To download the project, you can either go to the main page of the GitHub repository (https://github.com/bloodwiing/AboutUsUni), click the green `Code` button and then press on `Download ZIP`. Don't forget to extract it.<br>
Alternatively you can download the project via the `git` command line:

```shell
git clone https://github.com/bloodwiing/AboutUsUni.git
```

## Compiling
Open the project folder up and double-click the file `build.bat` (on Windows) or `build.sh` (on Linux).

A `main.exe` (on Windows) or `main.out` (on Linux) should appear.

## Running
Run the respective `exe` or `out` file that appeared while compiling.

You will be taken to a screen where you will be prompted to enter data, such as the company name, description, team size, etc.

***

### For the questions that ask a link
Make sure that the link is valid and displays an image on your browser if you go to it

***

### For the question that ask a colour
Make sure that the colour follows a HEX RGB format.

Meaning the input should consist of 6 hexadecimal (0-9,A-F) digits.

**Do not add any extra characters! You do not need a hashtag (#) in the input!**

You may use this [tool by webfx](https://www.webfx.com/web-design/color-picker/) to find a colour you would like.<br>
Once you choose a colour, you may simply copy the text _next_ to the hashtag (#) - those 6 symbols are your colour code that you can just simply paste into the program.<br>

***

### Number of team members must be positive
Due to the design of the website, we cannot allow 0 team members to be set - you need **at least** one.

## Hosting
When you finish an `out` folder should appear that contains an `index.html` and a `style.css`.<br>
Please copy these files and transfer them to the computer that hosts your website - you do not need to make any other changes.

If you want the page to be called differently and not be your main site page, please just rename the `index.html` file to something else (possibly `about.html`)

**Do not rename the `style.css` file however as that will break many things!**