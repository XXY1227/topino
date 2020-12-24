# Topino: A graphical tool for quantitative assessment of molecular stream separations

![](doc/topinologo.png)

## Introduction

In molecular-stream separation (MSS), a stream of a multi-component mixture is separated into multiple streams of individual components inside a thin rectangular chamber. Despite great potential and many years of work on MSS, its analysis was underdeveloped until recently. To fill in this gap, we introduced a novel and convenient way to assess MSS by convoluting the separation zone into a simple 2D plot called angulagram. We implemented and publicly shared Python programs for the generation of angulagrams. However, we realized that Python programs create two hurdles (setting up a resembling Python environment and using a command line interface) for other researchers to try out and adopt this new approach. To make our approach more accessible to the MSS community, we introduce and present here the open-source software Topino for quantitative assessment of MSS. Topino is a user-friendly, accessible program with a graphical user interface that allows the user to assess MSS data in a fast and straightforward way (less than 2\ min). 

## Installation

Topino is available for Windows (with and without installer) as well as Debian package. Please see [releases](https://github.com/Schallaven/topino/releases) for the latest (stable) version!

## Quick start

1. Import your MSS image by _File_ -> _Import image..._.
2. Define your inlet using the _Inlet tool_ ![](topino/ui/toolicons/inlet.png) and adjust the size.
3. Preprocess your image ![](topino/ui/toolicons/image_edit.png) by adjusting the colour levels to obtain maximum stream-background contrast.
4. Proceed to the _Show and edit the angulagram_ ![](topino/ui/toolicons/angulagram.png) view.
5. Evaluate the angulagram ![](topino/ui/toolicons/angulagram_evaluate.png) until you get the right number of peaks (1 peak per stream) by adjusting smoothing and threshold parameters.
6. Export the raw data ![](topino/ui/toolicons/angulagram_exportdata.png) and the angulagram ![](topino/ui/toolicons/angulagram_exportgraph.png) as required.

## User guide

Whether it’s your first time analyzing Molecular Stream Separations (MMS) or something that you do regularly, please take time to read the [small user guide](doc/README.md) to familiarise yourself with _Topino_, a handy software specifically designed for MSS. Topino is a simple and intuitive software that will help you to analyze MSS pictures, extract raw data, and generate manuscript-grade figures.

## Contributions

Contributions are welcome!

While we are presenting here a working practical tool, we do not claim that our implementation of this tool is the best nor the only one. In fact, we foresee continuing development when other groups start adapting our approach.  Hereby, we explicitly invite the community to criticize, discuss, and further develop our angulagram approach, Topino, and its implementation.



