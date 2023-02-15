package;

#if lime

import lime.tools.PlatformTarget;
import lime.tools.ProjectHelper;
import lime.tools.ProjectXMLParser;
import lime.tools.HXProject;

import massive.sys.io.FileSys;
import sys.FileSystem;
import hxp.System;
import hxp.Path;
import hxp.Log;

import utils.PathUtils;

using StringTools;

@:access(lime.tools.HXProject)
class SpoopyProject {
    public var project(default, null):HXProject;
    public var platform(default, null):PlatformTarget;

    public function new() {
        project = new HXProject();
        project.architectures = [];
    }

    public function addTemplate(arg:String):Void {
        project.templatePaths.push(arg);
    }

    public function copyAndCreateTemplate(name:String, destination:String):Void {

        /*
        * Check to see if anything is wrong
        */
        if((name == null || name == "") || (destination == null || destination == "")) {
            return;
        }

        /*
        * Placeholder Info
        */
        var id = ["com", "example", name.replace(" ", "").toLowerCase()];
        var alphaNumeric = new EReg("[a-zA-Z0-9]", "g");

        /*
        * Project Info
        */
        var title = "";
        var packageName = id.join(".").toLowerCase();
        var version = "1.0.0";
        var company = "Company Name";
        var file = name.replace(" ", "");

        /*
        * Initialize Info
        */
        var capitalizeNext = true;

        for (i in 0...name.length)
        {
            if (alphaNumeric.match(name.charAt(i)))
            {
                if (capitalizeNext)
                {
                    title += name.charAt(i).toUpperCase();
                }
                else
                {
                    title += name.charAt(i);
                }

                capitalizeNext = false;
            }
            else
            {
                capitalizeNext = true;
            }
        }

        project.meta.title = title;
        project.meta.packageName = packageName;
        project.meta.version = version;
        project.meta.company = company;
        project.app.file = file;

        var context:Dynamic = {};

        context.title = title;
        context.packageName = packageName;
        context.version = version;
        context.company = company;
        context.file = file;

        ProjectHelper.recursiveSmartCopyTemplate(project, "project", destination + "/" + name, context);
    }

    public function xmlProject(path:String):Void {
        var projectFile:String = path + "/project.xml";

        var userDefines:Map<String, Dynamic> = new Map<String, Dynamic>();
        var includePaths :Array<String> = new Array<String>();

        if(!FileSystem.exists(projectFile)) {
            projectFile = path + "/Project.xml";
        }

        if(!FileSystem.exists(projectFile)) {
            Log.error("Could not find \"project.xml\", must have a project xml in your project");
        }

        if(Path.extension(projectFile) == "lime" || Path.extension(projectFile) == "nmml" || Path.extension(projectFile) == "xml") {
            project = new ProjectXMLParser(Path.withoutDirectory(projectFile), userDefines, includePaths);
        }else {
            Log.error("Could not process \"" + projectFile + "\"");
        }
    }

    public function replaceProjectNDLL(path:String, dep:String):Void {
        var location:String = platform.targetDirectory + "/bin/" + dep;

        if(!FileSystem.exists(path + "/" + dep)) {
            Log.error("Couldn't not find \"" + dep + "\", please use 'spoopy build_ndll <platform>' to initialize NDLL: (" + path + "/" + dep + ")");
        }

        var replacing:String = PathUtils.recursivelyFindFile(platform.targetDirectory + "/bin", dep);

        if(replacing == "") {
            return;
        }

        FileSystem.deleteFile(replacing);
        System.copyFile(path + "/" + dep, replacing);
    }

    public function targetPlatform(command:String):Void {
        var targetFlags = new Map<String, String>();

        switch (project.target) {
            case ANDROID:
                platform = new AndroidPlatform(command, project, targetFlags);

            case IOS:
                platform = new IOSPlatform(command, project, targetFlags);

            case WINDOWS:
                platform = new WindowsPlatform(command, project, targetFlags);

            case MAC:
                platform = new MacPlatform(command, project, targetFlags);

            case LINUX:
                platform = new LinuxPlatform(command, project, targetFlags);

            case FLASH:
                platform = new FlashPlatform(command, project, targetFlags);

            case HTML5:
                platform = new HTML5Platform(command, project, targetFlags);

            case EMSCRIPTEN:
                platform = new EmscriptenPlatform(command, project, targetFlags);

            case TVOS:
                platform = new TVOSPlatform(command, project, targetFlags);

            case AIR:
                platform = new AIRPlatform(command, project, targetFlags);

            default:
        }
    }
}
#end