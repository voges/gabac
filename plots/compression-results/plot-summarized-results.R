# Install libraries
#install.packages("dplyr")
#install.packages("ggplot2")

# Load libraries
library("dplyr", warn.conflicts = FALSE)
library("ggplot2")
library("tools")

filenames <- list("plot-summarized-results-input/mpeg-g.csv", "plot-summarized-results-input/cram.csv", "plot-summarized-results-input/deez.csv", "plot-summarized-results-input/cram-and-deez.csv")
dir.create("plot-summarized-results-output")

for (filename in filenames) {
    # Load the data
    message("Processing: ", filename)
    dataframe = read.csv(filename)

    # Compute a dataframe containing the medians
    tmp <- dataframe %>% dplyr::group_by(codec) %>% dplyr::summarize(median_compression_time = median(compression_time_s))
    tmp2 <- dataframe %>% dplyr::group_by(codec) %>% dplyr::summarize(median_compression_rank = median(compression_rank))
    tmp3 <- dataframe %>% dplyr::group_by(codec) %>% dplyr::summarize(median_compression_speed_rank = median(compression_speed_rank))
    medians <- tmp %>% dplyr::inner_join(tmp, by = "codec")
    medians <- medians %>% dplyr::inner_join(tmp2, by = "codec")
    medians <- medians %>% dplyr::inner_join(tmp3, by = "codec")

    # Compute a dataframe containing the means
    tmp <- dataframe %>% dplyr::group_by(codec) %>% dplyr::summarize(mean_compression_time = mean(compression_time_s))
    tmp2 <- dataframe %>% dplyr::group_by(codec) %>% dplyr::summarize(mean_compression_rank = mean(compression_rank))
    tmp3 <- dataframe %>% dplyr::group_by(codec) %>% dplyr::summarize(mean_compression_speed_rank = mean(compression_speed_rank))
    means <- tmp %>% dplyr::inner_join(tmp, by = "codec")
    means <- means %>% dplyr::inner_join(tmp2, by = "codec")
    means <- means %>% dplyr::inner_join(tmp3, by = "codec")

    # Log some data
    #print(dataframe)
    #print(medians)
    #print(means)

    # Do not generate Rplots.pdf
    pdf(NULL)

    # Color blind-friendly palette
    cbPalette <- c("#56B4E9", "#009E73", "#999999", "#E69F00", "#F0E442", "#0072B2", "#D55E00", "#CC79A7")

    # Compression ratio versus compression speed
    plot_basename = paste(tools::file_path_sans_ext(basename(filename)), "compression-ratio-ranks-vs-compression-speed-ranks", sep = '-')
    plot_basename = paste("plot-summarized-results-output/", plot_basename, sep = '')
    pdf_plot_filename = paste(plot_basename, ".pdf", sep = '')
    png_plot_filename = paste(plot_basename, ".png", sep = '')
    message("Plotting compression ratio versus compression speed\n  to: ", pdf_plot_filename, "\n  to: ", png_plot_filename)
    ggplot2::ggplot(dataframe, aes(x = compression_speed_rank, y = compression_rank, color = codec)) +
      ggplot2::geom_jitter(aes(color = as.factor(codec)), size = 1.5, width = 0.15, height = 0.15, alpha = 0.2) +
      ggplot2::geom_point(data = medians, aes(x = median_compression_speed_rank, y = median_compression_rank), size = 7, shape = 18) +
      ggplot2::scale_x_continuous(breaks = c(1:6), labels = c(1:6)) +
      ggplot2::scale_y_reverse(breaks = c(1:6), labels = c(1:6)) +
      ggplot2::guides(color = guide_legend(title = "Codec")) +
      ggplot2::theme_light() +
      ggplot2::scale_fill_manual(values = cbPalette) +
      ggplot2::scale_colour_manual(values = cbPalette) +
      ggplot2::labs(x = "Rank of compression speed performance", y = "Rank of compression performance") +
      ggplot2::ggsave(file = pdf_plot_filename, width = 18, height = 18, units = "cm") +
      ggplot2::ggsave(file = png_plot_filename, width = 18, height = 18, units = "cm")

    # Compression ranks
    plot_basename = paste(tools::file_path_sans_ext(basename(filename)), "compression-ranks", sep = '-')
    plot_basename = paste("plot-summarized-results-output/", plot_basename, sep = '')
    pdf_plot_filename = paste(plot_basename, ".pdf", sep = '')
    png_plot_filename = paste(plot_basename, ".png", sep = '')
    message("Plotting compression ranks\n  to: ", pdf_plot_filename, "\n  to: ", png_plot_filename)
    ggplot2::ggplot(dataframe, aes(x = as.factor(test_set), y = compression_rank)) +
      ggplot2::geom_boxplot(aes(fill = as.factor(test_set)), outlier.shape = NA, alpha = 0.3) +
      ggplot2::geom_jitter(aes(color = as.factor(test_set)), size = 1, width = 0.13, height = 0.13, alpha = 0.3) +
      ggplot2::geom_hline(data = means, aes(yintercept = mean_compression_rank), linetype = "dashed", color = "tomato", size = 1) +
      ggplot2::scale_x_discrete(labels = function(x) stringr::str_pad(x, width = 2, pad = "0")) +
      ggplot2::scale_y_reverse(breaks = c(1:6), labels = c(1:6)) +
      ggplot2::facet_grid(cols = vars(codec)) +
      ggplot2::theme_light() +
      ggplot2::scale_fill_manual(values = cbPalette) +
      ggplot2::scale_colour_manual(values = cbPalette) +
      ggplot2::theme(legend.position = "none") +
      ggplot2::labs(x = "Test Set ID", y = "Rank") +
      ggplot2::ggsave(file = pdf_plot_filename, width = 18, height = 10, units = "cm") +
      ggplot2::ggsave(file = png_plot_filename, width = 18, height = 10, units = "cm")

    # Compression speed ranks
    plot_basename = paste(tools::file_path_sans_ext(basename(filename)), "compression-speed-ranks", sep = '-')
    plot_basename = paste("plot-summarized-results-output/", plot_basename, sep = '')
    pdf_plot_filename = paste(plot_basename, ".pdf", sep = '')
    png_plot_filename = paste(plot_basename, ".png", sep = '')
    message("Plotting compression speed ranks\n  to: ", pdf_plot_filename, "\n  to: ", png_plot_filename)
    ggplot2::ggplot(dataframe, aes(x = as.factor(test_set), y = compression_speed_rank)) +
      ggplot2::geom_boxplot(aes(fill = as.factor(test_set)), outlier.shape = NA, alpha = 0.3) +
      ggplot2::geom_jitter(aes(color = as.factor(test_set)), size = 1, width = 0.13, height = 0.13, alpha = 0.3) +
      ggplot2::geom_hline(data = means, aes(yintercept = mean_compression_speed_rank), linetype = "dashed", color = "tomato", size = 1) +
      ggplot2::scale_x_discrete(labels = function(x) stringr::str_pad(x, width = 2, pad = "0")) +
      ggplot2::scale_y_reverse(breaks = c(1:6), labels = c(1:6)) +
      ggplot2::facet_grid(cols = vars(codec)) +
      ggplot2::theme_light() +
      ggplot2::scale_fill_manual(values = cbPalette) +
      ggplot2::scale_colour_manual(values = cbPalette) +
      ggplot2::theme(legend.position = "none") +
      ggplot2::labs(x = "Test Set ID", y = "Rank") +
      ggplot2::ggsave(file = pdf_plot_filename, width = 18, height = 10, units = "cm") +
      ggplot2::ggsave(file = png_plot_filename, width = 18, height = 10, units = "cm")

    # Decompression speed ranks
    plot_basename = paste(tools::file_path_sans_ext(basename(filename)), "decompression-speed-ranks", sep = '-')
    plot_basename = paste("plot-summarized-results-output/", plot_basename, sep = '')
    pdf_plot_filename = paste(plot_basename, ".pdf", sep = '')
    png_plot_filename = paste(plot_basename, ".png", sep = '')
    message("Plotting decompression speed ranks\n  to: ", pdf_plot_filename, "\n  to: ", png_plot_filename)#
    ggplot2::ggplot(dataframe, aes(x = as.factor(test_set), y = decompression_speed_rank)) +
      ggplot2::geom_boxplot(aes(fill = as.factor(test_set)), outlier.shape = NA, alpha = 0.3) +
      ggplot2::geom_jitter(aes(color = as.factor(test_set)), size = 1, width = 0.13, height = 0.13, alpha = 0.3) +
      ggplot2::geom_hline(data = means, aes(yintercept = mean_compression_speed_rank), linetype = "dashed", color = "tomato", size = 1) +
      ggplot2::scale_x_discrete(labels = function(x) stringr::str_pad(x, width = 2, pad = "0")) +
      ggplot2::scale_y_reverse(breaks = c(1:6), labels = c(1:6)) +
      ggplot2::facet_grid(cols = vars(codec)) +
      ggplot2::theme_light() +
      ggplot2::scale_fill_manual(values = cbPalette) +
      ggplot2::scale_colour_manual(values = cbPalette) +
      ggplot2::theme(legend.position = "none") +
      ggplot2::labs(x = "Test Set ID", y = "Rank") +
      ggplot2::ggsave(file = pdf_plot_filename, width = 18, height = 10, units = "cm") +
      ggplot2::ggsave(file = png_plot_filename, width = 18, height = 10, units = "cm")
}
