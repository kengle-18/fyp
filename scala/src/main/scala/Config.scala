package com.example

import scala.io.Source
import java.io.File

object Config {

  private var _args: Array[String] = Array.empty
  private var _fileConfig: Map[Int, String] = Map.empty
  private val configDir: String = sys.env.getOrElse("CONFIG_DIR", "/scala-dest-test")
  private val configFile: String = sys.env.getOrElse("CONFIG_FILE", "config.txt")

  /*
  Select config file:
  1. If CONFIG_DIR contains a non-empty file other than CONFIG_FILE,only able to pick one file
  2. Else if CONFIG_FILE exists and is non-empty, use that
  3. Else error out*/

  private def findDefaultConfigFile(): String = {
    val dir = new File(configDir)

    if (!dir.exists() || !dir.isDirectory)
      throw new IllegalArgumentException(s"Config directory $configDir not found")

    val otherFileOpt = dir
      .listFiles()
      .filter(f => f.isFile && f.length() > 0 && f.getName != configFile)
      .headOption

    otherFileOpt.map(_.getAbsolutePath).getOrElse {
      // Fallback: try CONFIG_FILE
      val defaultFile = new File(configDir, configFile)
      if (defaultFile.exists() && defaultFile.length() > 0)
        defaultFile.getAbsolutePath
      else
        throw new IllegalArgumentException(
          s"No valid config file found in $configDir (including $configFile)"
        )
    }
  }

  /** Initialize with CLI args and optional config file path */
  def init(args: Array[String], configPath: String = findDefaultConfigFile()): Unit = {
    _args = args
    loadFileConfig(configPath)
  }

  /** Load config from file: each line = default for argument index */
  private def loadFileConfig(filePath: String): Unit = {
    val file = new File(filePath)
    if (file.exists()) {
      val lines = Source.fromFile(file).getLines().toList
      _fileConfig = lines.zipWithIndex.map { case (v, i) => i -> v.trim }.toMap
    }
  }

  /** Get single argument by index: CLI > config file > fallback */
  def getArg(index: Int, fallback: String = "Default"): String =
    if (_args.length > index && _args(index).nonEmpty) _args(index)
    else _fileConfig.getOrElse(index, fallback)

  /** Get all arguments merged: CLI > config > fallback */
  def getAll(fallbacks: Seq[String] = Seq.empty): Seq[String] = {
    val maxLength = math.max(_args.length, math.max(_fileConfig.size, fallbacks.length))
    (0 until maxLength).map(i => getArg(i, if (i < fallbacks.length) fallbacks(i) else ""))
  }
}
