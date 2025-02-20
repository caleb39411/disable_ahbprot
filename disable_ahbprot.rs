use std::fs::OpenOptions;
use std::io::{Read, Seek, SeekFrom, Write};
use std::process::exit;
use clap::{Arg, Command};

const PATTERN: &[u8] = &[0xD0, 0x0B, 0x23, 0x08, 0x43, 0x13, 0x60, 0x0B];
const PATCH: &[u8] = &[0x46, 0xC0, 0x23, 0x08, 0x43, 0x13, 0x60, 0x0B];

fn patch_file(filename: &str, verbose: bool) -> bool {
    let mut file = OpenOptions::new()
        .read(true)
        .write(true)
        .open(filename)
        .unwrap_or_else(|err| {
            eprintln!("Error opening file {}: {}", filename, err);
            exit(1);
        });

    let mut buffer = Vec::new();
    file.read_to_end(&mut buffer).unwrap_or_else(|err| {
        eprintln!("Error reading file {}: {}", filename, err);
        exit(1);
    });

    if let Some(pos) = buffer.windows(PATTERN.len()).position(|window| window == PATTERN) {
        file.seek(SeekFrom::Start(pos as u64)).unwrap();
        file.write_all(PATCH).unwrap_or_else(|err| {
            eprintln!("Error writing to file {}: {}", filename, err);
            exit(1);
        });

        if verbose {
            println!("Patch applied at offset: {}", pos);
        }
        return true;
    }

    if verbose {
        eprintln!("Pattern not found in file: {}", filename);
    }
    false
}

fn main() {
    let matches = Command::new("disable_ahbprot")
        .version("1.0")
        .about("Patches a file to disable AHBPROT")
        .arg(Arg::new("file")
            .required(true)
            .help("File to patch"))
        .arg(Arg::new("verbose")
            .short('v')
            .long("verbose")
            .help("Enable verbose output")
            .action(clap::ArgAction::SetTrue))
        .get_matches();

    let filename = matches.get_one::<String>("file").unwrap();
    let verbose = matches.get_flag("verbose");

    if verbose {
        println!("Patching file: {}", filename);
    }

    if !patch_file(filename, verbose) {
        eprintln!("Failed to patch file.");
        std::process::exit(1);
    }
}