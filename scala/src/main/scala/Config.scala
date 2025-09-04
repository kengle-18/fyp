package com.example

import scala.io.Source
import java.io.File

object Config {

  private var _args: Array[String] = Array.empty
  private var _fileConfig: Map[Int, String] = Map.empty

  /** Initialize with CLI args and optional config file path */
  def init(args: Array[String], configPath: String = "config.txt"): Unit = {
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
  def getArg(index: Int, fallback: String = ""): String =
    if (_args.length > index && _args(index).nonEmpty) _args(index)
    else _fileConfig.getOrElse(index, fallback)

  /** Get all arguments merged: CLI > config > fallback */
  def getAll(fallbacks: Seq[String] = Seq.empty): Seq[String] = {
    val maxLength = math.max(_args.length, math.max(_fileConfig.size, fallbacks.length))
    (0 until maxLength).map(i => getArg(i, if (i < fallbacks.length) fallbacks(i) else ""))
  }
}
