# Install libraries
#install.packages("ggplot2")

# Load libraries
library("ggplot2")
library("tools")

filenames <- list("plot-results-by-stream-input/cram-ERR17431.csv", "plot-results-by-stream-input/cram-NA12878.csv", "plot-results-by-stream-input/deez-ERR17431.csv", "plot-results-by-stream-input/deez-NA12878.csv")
dir.create("plot-results-by-stream-output")

for (filename in filenames) {
    # Load the data
    message("Processing: ", filename)
    dataframe = read.csv(filename)

    # Do not generate Rplots.pdf
    pdf(NULL)

    # Color blind-friendly palette
    cbPalette <- c("#56B4E9", "#009E73", "#999999", "#E69F00", "#F0E442", "#0072B2", "#D55E00", "#CC79A7")

    # Compression ratios
    plot_basename = paste(tools::file_path_sans_ext(basename(filename)), "compression-ratios", sep = '-')
    plot_basename = paste("plot-results-by-stream-output/", plot_basename, sep = '')
    pdf_plot_filename = paste(plot_basename, ".pdf", sep = '')
    png_plot_filename = paste(plot_basename, ".png", sep = '')
    message("Plotting compression ratios  \n  to: ", pdf_plot_filename, "\n  to: ", png_plot_filename)
    ggplot2::ggplot(dataframe) +
      ggplot2::geom_col(aes(x = codec, y = compression_ratio, fill = codec), alpha = 0.7) +
      ggplot2::facet_grid(cols = vars(stream_id)) +
      ggplot2::theme_light() +
      ggplot2::theme(axis.text.x = element_blank(), legend.position="bottom") +
      ggplot2::guides(fill = guide_legend(nrow = 1)) +
      ggplot2::labs(fill = "Codec", x = "", y = "Compression Ratio") +
      ggplot2::scale_fill_manual(values = cbPalette) +
      ggplot2::scale_colour_manual(values = cbPalette) +
      ggplot2::scale_y_continuous(expand = c(0, 0), limits = c(0, 1.1)) +
      ggplot2::ggsave(file = pdf_plot_filename, width = 100, height = 30, units = "cm") +
      ggplot2::ggsave(file = png_plot_filename, width = 100, height = 30, units = "cm")
}
