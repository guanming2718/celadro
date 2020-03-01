/*
 * This file is part of CELADRO, Copyright (C) 2016-20, Romain Mueller
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "header.hpp"
#include "model.hpp"
#include "files.hpp"

using namespace std;

void Model::WriteFrame(unsigned t)
{
  // construct output name
  const string oname = inline_str(output_dir, "frame", t, ".json");

  // write
  {
    stringstream buffer;
    {
      oarchive ar(buffer, "frame", 1);
      // serialize
      SerializeFrame(ar);

      if(ar.bad_value()) throw error_msg("nan found while writing file.");
    }
    // dump to file
    std::ofstream ofs(oname.c_str(), ios::out);
    ofs << buffer.rdbuf();
  }

  // compress
  if(compress) compress_file(oname, oname);
  if(compress_full) compress_file(oname, runname);
}

void Model::WriteParams()
{
  // a name that makes sense
  const string oname = inline_str(output_dir, "parameters.json");

  // write
  {
    stringstream buffer;
    {
      // serialize
      oarchive ar(buffer, "parameters", 1);
      // ...program parameters...
      ar & auto_name(Size)
         & auto_name(BC)
         & auto_name(nsteps)
         & auto_name(nsubsteps)
         & auto_name(ninfo)
         & auto_name(nstart);
      // ...and model parameters
      SerializeParameters(ar);

      if(ar.bad_value()) throw error_msg("nan found while writing file.");
    }
    // dump to file
    std::ofstream ofs(oname.c_str(), ios::out);
    ofs << buffer.rdbuf();
  }

  // compress
  if(compress) compress_file(oname, oname);
  if(compress_full) compress_file(oname, runname);
}

void Model::ClearOutput()
{
  if(compress_full)
  {
    // file name of the output file
    const string fname = runname + ".zip";

    {
      // try open it
      ifstream infile(fname);
      // does not exist we are fine
      if(not infile.good()) return;
    }

    if(not force_delete)
    {
      // ask
      char answ = 0;
      cout << " remove output file '" << fname << "'? ";
      cin >> answ;

      if(answ != 'y' and answ != 'Y')
        throw error_msg("output file '", fname,
                        "' already exist, please provide a different name.");
    }

    // delete
    remove_file(fname);
  }
  else
  {
    // extension of single files
    string ext = compress ? ".json.zip" : ".json";

    // check that parameters.json does not exist in the output dir and if it
    // does ask for deletion (this is not completely fool proof, but ok...)
    {
      ifstream infile(output_dir + "parameters" + ext);
      if(not infile.good()) return;
    }

    if(not force_delete)
    {
      // ask
      char answ = 0;
      cout << " remove output files in directory '" << output_dir << "'? ";
      cin >> answ;

      if(answ != 'y' and answ != 'Y')
        throw error_msg("output files already exist in directory '",
                        output_dir, "'.");
    }

    // delete all output files
    remove_file(output_dir);
  }
}

void Model::CreateOutputDir()
{
  // if full compression is on: we need to create a random tmp directory
  if(compress_full)
  {
    // use hash of runname string plus salt
    hash<string> hash_fn;
    unsigned dir_name = hash_fn(inline_str(runname, random_unsigned()));
    output_dir = inline_str("/tmp/", dir_name, "/");
  }
  // if full compression is off: just dump files where they belong
  else
    // note that runname can not be empty from options.cpp
    output_dir = runname + ( runname.back()=='/' ? "" : "/" );

  // clear output if needed
  ClearOutput();

  // create output dir if needed
  create_directory(output_dir);
}
