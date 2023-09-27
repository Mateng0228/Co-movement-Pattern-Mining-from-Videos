<h2 align="center">Co-movement Pattern Mining from Videos</h2>

<p align="center">
  <img src="teaser.png" alt="teaser" width="35%">
</p>

This project provides the source code and datasets for our paper  "Co-movement Pattern Mining from Videos." The discovery of co-movement patterns involves identifying all groups of objects that move within spatial proximity for a specified time interval.
In the paper mentioned above, we expand the meanings of co-movement patterns by migrating the data source from GPS sensors to surveillance cameras and conduct the first investigation into mining co-movement patterns from videos.

## Structure
The core of this project revolves around the TCS-tree and its related optimization code. Additionally, it includes the two baseline algorithms and the datasets mentioned in the paper. The primary structure of this project can be summarized as follows:

```shell
Co-movement-Pattern-Mining-from-Videos
   |——————SequenceAheadMining # TCS-tree and related code
   |         └——————algorithm
   |         └——————datasets
   |         └——————results
   |         └——————*.h
   |         └——————main.cpp
   └——————CMCBaseline # CMC algorithm
   └——————AprioriBaseline # Apriori based enumerator
   └——————Datasets
             └——————singapore
             └——————chengdu
             └——————Carla
```

> Note that SequenceAheadMining, CMCBaseline, and AprioriBaseline are three separate subprojects that are compiled and run independently.


## Requirements
- **Operating System:** Linux
- **C++ Standard Requirement:** requires a minimum of C++14 for compatibility with features like constexpr and variadic templates. 
- **Compiler Requirements:** GCC 11.4 or later (for support of GCC-specific extensions)
- **Build Tools Requirement:**
  - **CMake:** Version 3.22 or later is required for configuring the build process.
  - **Make:** A compatible version 4.3 of the 'make' tool is required to build the project.

## Installation
> Before starting, please ensure that you have installed and met all the requirements mentioned in the previous section.

You can quickly compile and install any of the subprojects in this project by following these steps (using "SequenceAheadMining" as an example):
1. **Clone the repository to your local machine:**
```shell
git clone https://github.com/Mateng0228/Co-movement-Pattern-Mining-from-Videos.git
```
2. **Change to the directory of subproject you want to use:**
```shell
cd Co-movement-Pattern-Mining-from-Videos/SequenceAheadMining
```
3. **Create a 'build' directory and change to it:**
```shell
mkdir build
cd build
```
4. **Compile current project using CMake and Make:**
```shell
cmake ..
make
```
5. **Ensure the "datasets" folder in subproject contains the dataset you want to use:**
```shell
cd ../../Datasets/
unzip "dataset-name.zip" # if necessary
cp -r "dataset-name" ../SequenceAheadMining/datasets/
cd ../SequenceAheadMining/
```
Now you can find and run the final executable file in the corresponding "build" directory according to the instructions in our "Usage and Reproduction" section later on.

## Usage and Reproduction
The three subprojects in this project have a similar command-line parameter interface. You can correctly configure and run the programs in the following format:
```shell
Executable-File "dataset-name" "m" "k" "epsilon" [OPTIONS]
```
- Note that you must switch to the root directory of the current subproject before running the executable file (due to relative path addressing in the code). If the executable file is located in the corresponding "build" directory, it means you should run the program using the format of "build/file-name".
- Again, please ensure that the "datasets" directory of the current subproject contains the "dataset-name" directory corresponding to your dataset.
- The meanings of the parameters "m," "k," and "eps" are consistent with those in the paper.
- The two baseline algorithms do not have any additional [OPTIONS] parameters. However, for our TCS algorithm, the additional [OPTIONS] parameters are as follows:
  - **no-tcs**: remove all optimizations related to the TCS-tree
  - **cmc**: substitute the sliding-window-based verification with CMC-like algorithm
  - **no-hash**: substitute the hashing-based dominance eliminator with one regular deduplication method
  - **details**: additionally output detailed pattern results to "results/output.csv"

After the program runs successfully, it will output the total execution time of the algorithm and the number of pattern results found.  
### Additional Notes on Reproduction
The structure of "singapore" and "chengdu" datasets is relatively straightforward. In their uncompressed directories, each text file represents the travel path of an individual vehicle. Regarding the "Carla" dataset, we have included supplementary information about its organization and usage in the readme.txt file within the dataset folder.  
Additionally, for precise parameter settings and comparative methods employed in the experiments presented in our paper, please consult the "Experiments" section of our original paper.  
By configuring the command-line parameters of the TCS algorithm and baseline algorithms appropriately, along with using the provided datasets, we believe readers should be able to reproduce the experimental results in the paper.

## Examples
**Here are some specific usage examples (still using SequenceAheadMining for demonstration):**

1. Run the TCS algorithm on the "singapore" dataset, evaluate the detailed execution time, and write the results to "result/output.csv".
- **Input**:
```shell
build/Project singapore 3 5 60 details
```
- **Output**:
```shell
parameters{dataset:singapore, m:3, k:5, epsilon:60}, additional options[ details ]:
cluster: 2.22219 | build tree: 0.246959 | verification: 0.25313 | de_duplicate: 0.076563 | Total elapsed time: 4.09293s
```

2. Run the TCS algorithm on the "chengdu" dataset without using the sliding-window-based verification.
- **Input**:
```shell
build/Project chengdu 3 5 60 cmc
```
- **Output**:
```shell
parameters{dataset:chengdu, m:3, k:5, epsilon:60}, additional options[ cmc ]:
cluster: 1.23426 | build tree: 1.26664 | verification: 8.81589 | de_duplicate: 0.458421 | Total number of discovered convoys: 4327, object combinations: 4076 - Total elapsed time: 13.8591s
```
**The following example demonstrates the process of reproducing the "algorithm runtime w.r.t path length k" part in Section 6.2 "Scalability Analysis" of our paper:**  
- **Preparation Phase**
  - Compile the three sub-projects "SequenceAheadMining", "CMCBaseline" and "AprioriBaseline" according to the "Installation" section instructions to generate executable files in each sub-project's "build" directory.
  - Place the datasets "singapore" and "chengdu" into the "datasets" directory of each sub-project.  

  Now the current main project structure is as follows (with some omissions):
  ```shell
  Co-movement-Pattern-Mining-from-Videos
     |——————SequenceAheadMining
     |         └——————datasets
     |         └——————build
     |                  └——————Project # executable file
     |——————CMCBaseline
     |         └——————datasets
     |         └——————build
     |                  └——————cmc # executable file
     |——————AprioriBaseline
     |         └——————datasets
     |         └——————build
     |                  └——————ConvoyBaseline # executable file
     └——————Datasets
  ```
- **Testing Phase**
  - Run the "TCS-tree" algorithm in "SequenceAheadMining" sub-project using commands with the form:  
  ```build/Project "dataset-name" 3 "k" 60```  
  - Run the "FSM" algorithm in the "SequenceAheadMining" sub-project. The commands are similar to the previous one but includes additional OPTION settings.  
  ```build/Project "dataset-name" 3 "k" 60 no-tcs cmc no-hash ```  
  - Run the "CMC" algorithm in "CMCBaseline" sub-project using commands with the form:  
  ```build/cmc "dataset-name" 3 "k" 60``` 
  - Run the "Apriori" algorithm in "AprioriBaseline" sub-project using commands with the form:  
  ```build/ConvoyBaseline "dataset-name" 3 "k" 60```  

  Due to the typically slower runtime of "CMC" and "Apriori" algorithms, we recommend utilizing relevant script files to control program execution.
- **Data Collection and Plotting Phase**  
Collect the data related to the final runtime of the algorithms obtained during the "Testing Phase", then use a  visualization tool to plot relevant charts:

<p align="center">
  <img src="example.png" alt="teaser" width="75%">
</p>

**Through a similar approach, we believe the process of reproducing other experiments in the paper is clear.**

## Contact
If you encounter any problems, please contact the code maintainer at [mt0228@zju.edu.cn](mailto:mt0228@zju.edu.cn). Please note that simply raising "Issues" in github may not always be the most effective way to get our attention.
